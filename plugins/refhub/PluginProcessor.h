#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>

namespace ehvota::refhub {

class RefHubAudioProcessor final : public juce::AudioProcessor {
public:
    RefHubAudioProcessor();
    ~RefHubAudioProcessor() override = default;

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

    float livePeakDb() const noexcept { return livePeakDb_.load(std::memory_order_relaxed); }
    float liveRmsDb() const noexcept { return liveRmsDb_.load(std::memory_order_relaxed); }

private:
    std::atomic<float> livePeakDb_ { -100.0f };
    std::atomic<float> liveRmsDb_ { -100.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefHubAudioProcessor)
};

} // namespace ehvota::refhub
