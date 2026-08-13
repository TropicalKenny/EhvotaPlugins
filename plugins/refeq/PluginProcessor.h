#pragma once

#include "shared/audio/Biquad.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>

namespace ehvota::refeq {

class RefEQAudioProcessor final : public juce::AudioProcessor {
public:
    RefEQAudioProcessor();
    ~RefEQAudioProcessor() override = default;

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
    struct SmoothValue {
        double current {};
        bool initialised {};

        double follow(double target, double amount) noexcept
        {
            if (!initialised) {
                current = target;
                initialised = true;
            }
            else {
                current += (target - current) * amount;
            }
            return current;
        }

        void snap(double value) noexcept
        {
            current = value;
            initialised = true;
        }
    };

    struct BandState {
        ehvota::audio::StereoBiquad filter;
        SmoothValue frequency;
        SmoothValue gainDb;
        SmoothValue q;
        double activeGainDb {};
    };

    void updateFilters(int blockSamples) noexcept;
    [[nodiscard]] bool isNeutral() const noexcept;

    juce::AudioProcessorValueTreeState parameters_;
    ehvota::audio::StereoBiquad highPass_;
    ehvota::audio::StereoBiquad lowPass_;
    std::array<BandState, 6> bands_ {};
    SmoothValue highPassFrequency_;
    SmoothValue lowPassFrequency_;
    SmoothValue outputDb_;
    double activeOutputGain_ { 1.0 };
    double sampleRate_ { 44100.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefEQAudioProcessor)
};

} // namespace ehvota::refeq
