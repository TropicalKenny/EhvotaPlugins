#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <memory>

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

    bool loadReferenceFile(const juce::File& file);
    void unloadReferenceFile();
    void startReferenceAudition();
    void stopReferenceAudition();
    void setReferenceMonitorTrimDb(float trimDb) noexcept;
    void setRenderSafe(bool enabled) noexcept { renderSafe_.store(enabled, std::memory_order_release); }

    [[nodiscard]] bool hasReferenceLoaded() const noexcept { return referenceLoaded_.load(std::memory_order_acquire); }
    [[nodiscard]] bool referenceAuditionActive() const noexcept { return referenceAudition_.load(std::memory_order_acquire); }
    [[nodiscard]] float referenceMonitorTrimDb() const noexcept { return referenceTrimDb_.load(std::memory_order_relaxed); }
    [[nodiscard]] bool renderSafe() const noexcept { return renderSafe_.load(std::memory_order_acquire); }

private:
    juce::AudioFormatManager formatManager_;
    std::unique_ptr<juce::AudioFormatReaderSource> referenceReaderSource_;
    juce::AudioTransportSource referenceTransport_;

    std::atomic<float> livePeakDb_ { -100.0f };
    std::atomic<float> liveRmsDb_ { -100.0f };
    std::atomic<float> referenceTrimDb_ { 0.0f };
    std::atomic<bool> referenceLoaded_ { false };
    std::atomic<bool> referenceAudition_ { false };
    std::atomic<bool> renderSafe_ { true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefHubAudioProcessor)
};

} // namespace ehvota::refhub
