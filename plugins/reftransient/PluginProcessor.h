#pragma once

#include "shared/audio/TransientShaper.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>

namespace ehvota::reftransient {

class RefTransientAudioProcessor final : public juce::AudioProcessor {
public:
    RefTransientAudioProcessor();
    ~RefTransientAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState& parameters() noexcept { return parameters_; }
    float transientActivity() const noexcept { return transientActivity_.load(std::memory_order_relaxed); }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::AudioProcessorValueTreeState parameters_;
    ehvota::audio::StereoTransientShaper shaper_;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> attack_;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sustain_;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sensitivity_;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputDb_;
    std::atomic<float> transientActivity_ {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefTransientAudioProcessor)
};

} // namespace ehvota::reftransient
