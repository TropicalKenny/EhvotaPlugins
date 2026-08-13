#pragma once

#include "plugins/common/RefLinkEndpoint.h"
#include "shared/analysis/ComparisonAnalysis.h"
#include "shared/audio/CaptureBuffer.h"
#include "shared/safety/ProposalPolicy.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <thread>

namespace ehvota::refhub {

enum class SectionAnalysisState : std::uint8_t {
    idle,
    running,
    proposalReady,
    noProposal,
    error
};

class RefHubAudioProcessor final : public juce::AudioProcessor, private juce::Timer {
public:
    RefHubAudioProcessor();
    ~RefHubAudioProcessor() override;

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

    [[nodiscard]] bool beginMixCapture(double durationSeconds) noexcept;
    void cancelMixCapture() noexcept;
    [[nodiscard]] ehvota::audio::CaptureState mixCaptureState() const noexcept { return mixCapture_.state(); }
    [[nodiscard]] double mixCaptureProgress() const noexcept { return mixCapture_.progress(); }
    [[nodiscard]] ehvota::audio::CaptureView completedMixCapture() const noexcept { return mixCapture_.completedView(); }

    // The mix and reference songs do not need matching absolute timelines. The caller supplies
    // the paired reference-section start; captured mix duration defines the comparison length.
    [[nodiscard]] bool startSectionAnalysis(
        ehvota::communication::SharedInstanceId refEqTarget,
        double referenceStartSeconds,
        std::string sectionId);
    [[nodiscard]] SectionAnalysisState sectionAnalysisState() const noexcept {
        return analysisState_.load(std::memory_order_acquire);
    }
    [[nodiscard]] std::optional<ehvota::analysis::SectionComparison> copyLatestComparison() const;

    [[nodiscard]] bool hasReferenceLoaded() const noexcept { return referenceLoaded_.load(std::memory_order_acquire); }
    [[nodiscard]] bool referenceAuditionActive() const noexcept { return referenceAudition_.load(std::memory_order_acquire); }
    [[nodiscard]] float referenceMonitorTrimDb() const noexcept { return referenceTrimDb_.load(std::memory_order_relaxed); }
    [[nodiscard]] bool renderSafe() const noexcept { return renderSafe_.load(std::memory_order_acquire); }

    [[nodiscard]] std::size_t linkedInstanceCount() const noexcept { return linkedCount_.load(std::memory_order_acquire); }
    [[nodiscard]] std::size_t copyLinkedInstances(
        std::array<ehvota::communication::SharedInstanceSnapshot,
            ehvota::communication::sharedLinkMaxInstances>& destination) const;
    [[nodiscard]] ehvota::communication::SharedLinkOpenStatus refLinkStatus() const noexcept { return refLink_.status(); }

    [[nodiscard]] bool stageRefEqProposal(
        const ehvota::analysis::SectionComparison& comparison,
        ehvota::communication::SharedInstanceId target,
        std::string evidenceId);
    [[nodiscard]] std::optional<ehvota::safety::Proposal> copyStagedProposal() const;
    void rejectStagedProposal();
    [[nodiscard]] bool approveAndSubmitStagedProposal();
    [[nodiscard]] bool submitApprovedProposal(
        const ehvota::safety::Proposal& proposal,
        ehvota::communication::SharedInstanceId target);

private:
    void timerCallback() override;
    void reconcileStagedProposal(
        const std::array<ehvota::communication::SharedInstanceSnapshot,
            ehvota::communication::sharedLinkMaxInstances>& snapshots,
        std::size_t count);
    void stopAnalysisThread() noexcept;

    juce::AudioFormatManager formatManager_;
    std::unique_ptr<juce::AudioFormatReaderSource> referenceReaderSource_;
    juce::AudioTransportSource referenceTransport_;
    juce::File referenceFile_;
    ehvota::audio::CaptureBuffer mixCapture_;

    std::atomic<float> livePeakDb_ { -100.0f };
    std::atomic<float> liveRmsDb_ { -100.0f };
    std::atomic<std::uint64_t> sampleCounter_ {};
    std::atomic<float> referenceTrimDb_ { 0.0f };
    std::atomic<bool> referenceLoaded_ { false };
    std::atomic<bool> referenceAudition_ { false };
    std::atomic<bool> renderSafe_ { true };

    std::jthread analysisThread_;
    std::atomic<bool> analysisRunning_ { false };
    std::atomic<SectionAnalysisState> analysisState_ { SectionAnalysisState::idle };
    mutable juce::CriticalSection comparisonLock_;
    std::optional<ehvota::analysis::SectionComparison> latestComparison_;

    ehvota::plugins::RefLinkEndpoint refLink_ {
        ehvota::communication::SharedPluginKind::refHub,
        ehvota::communication::RefLinkRole::master,
        "RefHub"
    };
    mutable juce::CriticalSection linkCacheLock_;
    std::array<ehvota::communication::SharedInstanceSnapshot,
        ehvota::communication::sharedLinkMaxInstances> linkedInstances_ {};
    std::atomic<std::size_t> linkedCount_ {};

    mutable juce::CriticalSection proposalLock_;
    std::optional<ehvota::safety::Proposal> stagedProposal_;
    ehvota::communication::SharedInstanceId stagedTarget_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefHubAudioProcessor)
};

} // namespace ehvota::refhub
