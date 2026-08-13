#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>

namespace ehvota::refcomp {

class RefCompAudioProcessor final : public juce::AudioProcessor {
public:
    RefCompAudioProcessor();
    ~RefCompAudioProcessor() override = default;

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
    float gainReductionDb() const noexcept { return gainReductionDb_.load(std::memory_order_relaxed); }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    double sampleRate_ { 44100.0 };
    float envelopeDb_ { -120.0f };
    std::atomic<float> gainReductionDb_ { 0.0f };
    juce::AudioProcessorValueTreeState parameters_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefCompAudioProcessor)
};

} // namespace ehvota::refcomp
