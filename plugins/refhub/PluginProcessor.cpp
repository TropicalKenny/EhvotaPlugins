#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "shared/safety/LinkProposalAdapter.h"

#include <cmath>
#include <span>

namespace ehvota::refhub {

RefHubAudioProcessor::RefHubAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    formatManager_.registerBasicFormats();
    startTimerHz(10);
}

RefHubAudioProcessor::~RefHubAudioProcessor() { stopTimer(); }

void RefHubAudioProcessor::prepareToPlay(const double sampleRate, const int samplesPerBlock)
{
    referenceTransport_.prepareToPlay(samplesPerBlock, sampleRate);
    mixCapture_.prepare(sampleRate, 60.0);
    sampleCounter_.store(0, std::memory_order_relaxed);
}
void RefHubAudioProcessor::releaseResources(){mixCapture_.cancel();referenceTransport_.releaseResources();}
bool RefHubAudioProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainInputChannelSet()==l.getMainOutputChannelSet()&&(l.getMainInputChannelSet()==juce::AudioChannelSet::mono()||l.getMainInputChannelSet()==juce::AudioChannelSet::stereo());}

bool RefHubAudioProcessor::loadReferenceFile(const juce::File& file){stopReferenceAudition();referenceTransport_.setSource(nullptr);referenceReaderSource_.reset();referenceLoaded_.store(false);auto reader=std::unique_ptr<juce::AudioFormatReader>(formatManager_.createReaderFor(file));if(!reader)return false;auto rate=reader->sampleRate;referenceReaderSource_=std::make_unique<juce::AudioFormatReaderSource>(reader.release(),true);referenceTransport_.setSource(referenceReaderSource_.get(),0,nullptr,rate,2);referenceTransport_.setPosition(0);referenceLoaded_.store(true);return true;}
void RefHubAudioProcessor::unloadReferenceFile(){stopReferenceAudition();referenceTransport_.setSource(nullptr);referenceReaderSource_.reset();referenceLoaded_.store(false);}
void RefHubAudioProcessor::startReferenceAudition(){if(!hasReferenceLoaded())return;if(referenceTransport_.hasStreamFinished())referenceTransport_.setPosition(0);referenceTransport_.start();referenceAudition_.store(true);}
void RefHubAudioProcessor::stopReferenceAudition(){referenceAudition_.store(false);referenceTransport_.stop();}
void RefHubAudioProcessor::setReferenceMonitorTrimDb(float db)noexcept{referenceTrimDb_.store(juce::jlimit(-24.f,24.f,db));}

void RefHubAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;float peak=0;double ss=0;std::uint64_t count=0;for(int ch=0;ch<buffer.getNumChannels();++ch){auto*d=buffer.getReadPointer(ch);for(int i=0;i<buffer.getNumSamples();++i){auto x=d[i];peak=juce::jmax(peak,std::abs(x));ss+=(double)x*x;++count;}}
    if(buffer.getNumChannels()>0&&buffer.getNumSamples()>0){auto left=std::span<const float>(buffer.getReadPointer(0),(std::size_t)buffer.getNumSamples());auto right=buffer.getNumChannels()>1?std::span<const float>(buffer.getReadPointer(1),(std::size_t)buffer.getNumSamples()):left;mixCapture_.push(left,right);}auto rms=count? (float)std::sqrt(ss/(double)count):0;livePeakDb_.store(juce::Decibels::gainToDecibels(peak,-100.f));liveRmsDb_.store(juce::Decibels::gainToDecibels(rms,-100.f));sampleCounter_.fetch_add((std::uint64_t)buffer.getNumSamples());
    bool protectedRender=renderSafe_.load()&&isNonRealtime();bool audition=referenceAudition_.load()&&referenceLoaded_.load()&&!protectedRender;if(audition){juce::AudioSourceChannelInfo info(&buffer,0,buffer.getNumSamples());referenceTransport_.getNextAudioBlock(info);buffer.applyGain(juce::Decibels::decibelsToGain(referenceTrimDb_.load()));if(referenceTransport_.hasStreamFinished())referenceAudition_.store(false);}
}

void RefHubAudioProcessor::timerCallback()
{
    auto peak=livePeakDb_.load(),rms=liveRmsDb_.load();refLink_.publish({peak,rms,1.0f,peak>-99&&rms>-99?peak-rms:0,sampleCounter_.load()});
    std::array<ehvota::communication::SharedInstanceSnapshot,ehvota::communication::sharedLinkMaxInstances> found{};auto n=refLink_.readActive(found);const juce::ScopedLock lock(linkCacheLock_);std::size_t kept=0;for(std::size_t i=0;i<n&&kept<linkedInstances_.size();++i){auto&x=found[i];if(x.pluginKind==ehvota::communication::SharedPluginKind::refHub||x.heartbeatAgeMs>5000)continue;linkedInstances_[kept++]=x;}for(std::size_t i=kept;i<linkedInstances_.size();++i)linkedInstances_[i]={};linkedCount_.store(kept,std::memory_order_release);
}

std::size_t RefHubAudioProcessor::copyLinkedInstances(std::array<ehvota::communication::SharedInstanceSnapshot,ehvota::communication::sharedLinkMaxInstances>&d)const{const juce::ScopedLock lock(linkCacheLock_);auto n=linkedCount_.load();for(std::size_t i=0;i<n;++i)d[i]=linkedInstances_[i];return n;}

bool RefHubAudioProcessor::submitApprovedProposal(const ehvota::safety::Proposal& proposal,const ehvota::communication::SharedInstanceId target)
{
    ehvota::communication::SharedPluginKind kind=ehvota::communication::SharedPluginKind::unknown;{
        const juce::ScopedLock lock(linkCacheLock_);const auto n=linkedCount_.load(std::memory_order_acquire);for(std::size_t i=0;i<n;++i){const auto&s=linkedInstances_[i];if(s.id.slot==target.slot&&s.id.generation==target.generation&&s.heartbeatAgeMs<=5000){kind=s.pluginKind;break;}}
    }
    if(kind==ehvota::communication::SharedPluginKind::unknown)return false;auto mailbox=ehvota::safety::encodeApprovedSharedProposal(proposal,target,kind);return mailbox.has_value()&&refLink_.submitProposal(target,*mailbox);
}

juce::AudioProcessorEditor*RefHubAudioProcessor::createEditor(){return new RefHubAudioProcessorEditor(*this);}void RefHubAudioProcessor::getStateInformation(juce::MemoryBlock&d){juce::MemoryOutputStream s(d,true);s.writeInt(3);s.writeFloat(referenceTrimDb_.load());s.writeBool(renderSafe_.load());}void RefHubAudioProcessor::setStateInformation(const void*d,int n){juce::MemoryInputStream s(d,(std::size_t)n,false);auto v=s.readInt();if(v>=2){setReferenceMonitorTrimDb(s.readFloat());setRenderSafe(s.readBool());}}
} // namespace ehvota::refhub
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ehvota::refhub::RefHubAudioProcessor();}
