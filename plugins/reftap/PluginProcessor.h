#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>

namespace ehvota::reftap {

class RefTapAudioProcessor final : public juce::AudioProcessor {
public:
    RefTapAudioProcessor();
    ~RefTapAudioProcessor() override = default;

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

    float peakDb() const noexcept { return peakDb_.load(std::memory_order_relaxed); }
    float rmsDb() const noexcept { return rmsDb_.load(std::memory_order_relaxed); }
    float correlation() const noexcept { return correlation_.load(std::memory_order_relaxed); }

private:
    std::atomic<float> peakDb_ { -100.0f };
    std::atomic<float> rmsDb_ { -100.0f };
    std::atomic<float> correlation_ { 1.0f };
    double sampleRate_ { 44100.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefTapAudioProcessor)
};

} // namespace ehvota::reftap
