#pragma once
#include "plugins/common/RefLinkEndpoint.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>
#include <array>
#include <atomic>
#include <cstdint>
namespace ehvota::refcomp {
class RefCompAudioProcessor final:public juce::AudioProcessor,private juce::Timer{
public: RefCompAudioProcessor();~RefCompAudioProcessor()override;void prepareToPlay(double,int)override;void releaseResources()override;bool isBusesLayoutSupported(const BusesLayout&)const override;void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;juce::AudioProcessorEditor*createEditor()override;bool hasEditor()const override{return true;}const juce::String getName()const override{return JucePlugin_Name;}bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}double getTailLengthSeconds()const override{return 0;}int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}const juce::String getProgramName(int)override{return{};}void changeProgramName(int,const juce::String&)override{}void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;juce::AudioProcessorValueTreeState&parameters()noexcept{return parameters_;}float gainReductionDb()const noexcept{return gainReductionDb_.load();}float activeAttackMs()const noexcept{return activeAttackMs_.load();}float activeReleaseMs()const noexcept{return activeReleaseMs_.load();}static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:void timerCallback()override;double sampleRate_{44100};std::array<float,2>envelopeDb_{-120,-120};std::atomic<float>gainReductionDb_{0},activeAttackMs_{20},activeReleaseMs_{180};std::atomic<std::uint64_t>sampleCounter_{};juce::AudioProcessorValueTreeState parameters_;ehvota::plugins::RefLinkEndpoint refLink_{ehvota::communication::SharedPluginKind::refComp,ehvota::communication::RefLinkRole::track,"RefComp"};JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefCompAudioProcessor)};
} // namespace
