#pragma once

#include "shared/audio/AnalogSaturation.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace ehvota::refcolor {

class RefColorAudioProcessor final : public juce::AudioProcessor {
public:
    RefColorAudioProcessor();
    ~RefColorAudioProcessor() override = default;

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
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    void updateSmoothingTargets() noexcept;

    juce::AudioProcessorValueTreeState parameters_;
    juce::dsp::Oversampling<float> oversampling_;
    ehvota::audio::DcBlocker dcBlocker_;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amount_;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveDb_;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputDb_;
    double sampleRate_ { 44100.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefColorAudioProcessor)
};

} // namespace ehvota::refcolor
