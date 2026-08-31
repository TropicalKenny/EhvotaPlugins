#pragma once

#include "plugins/common/RefLinkEndpoint.h"
#include "shared/audio/CaptureBuffer.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>

#include <atomic>
#include <cstdint>
#include <optional>
#include <thread>

namespace ehvota::reftap {

class RefTapAudioProcessor final : public juce::AudioProcessor, private juce::Timer {
public:
    RefTapAudioProcessor();
    ~RefTapAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void updateTrackProperties(const TrackProperties& properties) override;

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
    juce::String persistentInstanceId() const { return persistentInstanceId_; }

private:
    struct CaptureAnalysisResult {
        bool valid {};
        std::uint32_t resultCode {};
        ehvota::communication::SharedCapturedAnalysis analysis;
    };

    void timerCallback() override;
    void handleCaptureRequest();
    void handleCaptureProgress();
    void launchCaptureAnalysis();
    void stopCaptureAnalysis() noexcept;
    void publishPersistentIdentity();

    std::atomic<float> peakDb_ { -100.0f };
    std::atomic<float> rmsDb_ { -100.0f };
    std::atomic<float> correlation_ { 1.0f };
    std::atomic<float> crestDb_ {};
    std::atomic<std::uint64_t> sampleCounter_ {};
    double sampleRate_ { 44100.0 };

    ehvota::audio::CaptureBuffer capture_;
    std::atomic<std::int64_t> scheduledHostStart_ { -1 };
    std::atomic<bool> scheduleMissed_ { false };
    std::atomic<std::uint64_t> activeCaptureSequence_ {};
    std::atomic<std::uint64_t> activeCaptureId_ {};
    std::jthread captureAnalysisThread_;
    std::atomic<bool> captureAnalysisRunning_ { false };
    mutable juce::CriticalSection captureAnalysisLock_;
    std::optional<CaptureAnalysisResult> pendingCaptureAnalysis_;

    juce::String persistentInstanceId_;
    ehvota::plugins::RefLinkEndpoint refLink_ {
        ehvota::communication::SharedPluginKind::refTap,
        ehvota::communication::RefLinkRole::bus,
        "RefTap"
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefTapAudioProcessor)
};

} // namespace ehvota::reftap
