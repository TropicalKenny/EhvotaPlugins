#pragma once
#include "plugins/common/RefLinkEndpoint.h"
#include "shared/audio/TransientShaper.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>
#include <atomic>
#include <cstdint>
namespace ehvota::reftransient{
class RefTransientAudioProcessor final:public juce::AudioProcessor,private juce::Timer{
public:RefTransientAudioProcessor();~RefTransientAudioProcessor()override;void prepareToPlay(double,int)override;void releaseResources()override;bool isBusesLayoutSupported(const BusesLayout&)const override;void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;juce::AudioProcessorEditor*createEditor()override;bool hasEditor()const override{return true;}const juce::String getName()const override{return JucePlugin_Name;}bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}double getTailLengthSeconds()const override{return 0;}int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}const juce::String getProgramName(int)override{return{};}void changeProgramName(int,const juce::String&)override{}void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;juce::AudioProcessorValueTreeState&parameters()noexcept{return parameters_;}float transientActivity()const noexcept{return transientActivity_.load(std::memory_order_relaxed);}static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:void timerCallback()override;juce::AudioProcessorValueTreeState parameters_;ehvota::audio::StereoTransientShaper shaper_;juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear>attack_,sustain_,sensitivity_,outputDb_;std::atomic<float>transientActivity_{};std::atomic<std::uint64_t>sampleCounter_{};ehvota::plugins::RefLinkEndpoint refLink_{ehvota::communication::SharedPluginKind::refTransient,ehvota::communication::RefLinkRole::track,"RefTransient"};JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefTransientAudioProcessor)};
}
