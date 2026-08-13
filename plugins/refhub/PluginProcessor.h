#pragma once

#include "ComponentCaptureCoordinator.h"
#include "plugins/common/RefLinkEndpoint.h"
#include "shared/analysis/CapturedSectionAnalysis.h"
#include "shared/analysis/ComparisonAnalysis.h"
#include "shared/analysis/LevelMatch.h"
#include "shared/audio/CaptureBuffer.h"
#include "shared/safety/ProposalHistory.h"
#include "shared/safety/ProposalPolicy.h"
#include "shared/state/MappingIdentity.h"

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
#include <vector>

namespace ehvota::refhub {

enum class SectionAnalysisState : std::uint8_t {
    idle,
    running,
    complete,
    proposalReady,
    noProposal,
    stale,
    error
};

enum class RefHubLevelMatchMode : std::uint8_t {
    autoSection,
    wholeTrack,
    off
};

struct SectionAnalysisSnapshot {
    bool valid {};
    std::string sectionId;
    std::string evidenceId;
    std::uint64_t captureGeneration {};
    std::uint64_t referenceGeneration {};
    double referenceStartSeconds {};
    double referenceDurationSeconds {};
    ehvota::analysis::CapturedSectionAnalysis mix;
    ehvota::analysis::CapturedSectionAnalysis reference;
    ehvota::analysis::SectionComparison comparison;
    ehvota::analysis::LevelMatchDecision levelMatch;
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
    [[nodiscard]] juce::String savedReferencePath() const;

    [[nodiscard]] bool beginMixCapture(double durationSeconds) noexcept;
    void cancelMixCapture() noexcept;
    [[nodiscard]] ehvota::audio::CaptureState mixCaptureState() const noexcept { return mixCapture_.state(); }
    [[nodiscard]] double mixCaptureProgress() const noexcept { return mixCapture_.progress(); }
    [[nodiscard]] ehvota::audio::CaptureView completedMixCapture() const noexcept { return mixCapture_.completedView(); }

    [[nodiscard]] bool startSectionAnalysis(
        double referenceStartSeconds,
        double referenceDurationSeconds,
        std::string sectionId);
    [[nodiscard]] bool startSectionAnalysis(
        ehvota::communication::SharedInstanceId refEqTarget,
        double referenceStartSeconds,
        double referenceDurationSeconds,
        std::string sectionId);

    [[nodiscard]] SectionAnalysisState sectionAnalysisState() const noexcept { return analysisState_.load(std::memory_order_acquire); }
    [[nodiscard]] std::optional<SectionAnalysisSnapshot> copyLatestSectionAnalysis() const;
    [[nodiscard]] std::optional<ehvota::analysis::SectionComparison> copyLatestComparison() const;

    void setLevelMatchMode(RefHubLevelMatchMode mode);
    [[nodiscard]] RefHubLevelMatchMode levelMatchMode() const noexcept { return levelMatchMode_.load(std::memory_order_acquire); }
    void setBlindComparison(bool enabled);
    [[nodiscard]] bool blindComparison() const noexcept { return blindComparison_.load(std::memory_order_acquire); }
    [[nodiscard]] bool sectionLevelMatchReady() const;

    [[nodiscard]] bool hasReferenceLoaded() const noexcept { return referenceLoaded_.load(std::memory_order_acquire); }
    [[nodiscard]] bool referenceAuditionActive() const noexcept { return referenceAudition_.load(std::memory_order_acquire); }
    [[nodiscard]] float referenceMonitorTrimDb() const noexcept { return referenceTrimDb_.load(std::memory_order_relaxed); }
    [[nodiscard]] bool renderSafe() const noexcept { return renderSafe_.load(std::memory_order_acquire); }

    [[nodiscard]] std::size_t linkedInstanceCount() const noexcept { return linkedCount_.load(std::memory_order_acquire); }
    [[nodiscard]] std::size_t copyLinkedInstances(
        std::array<ehvota::communication::SharedInstanceSnapshot,
            ehvota::communication::sharedLinkMaxInstances>& destination) const;
    [[nodiscard]] ehvota::communication::SharedLinkOpenStatus refLinkStatus() const noexcept { return refLink_.status(); }

    [[nodiscard]] bool setComponentMapping(
        ehvota::state::ReferenceRole role,
        ehvota::communication::SharedInstanceId refTap);
    void clearComponentMapping(ehvota::state::ReferenceRole role);
    [[nodiscard]] std::array<RuntimeComponentMapping, 4> copyComponentMappings() const;
    [[nodiscard]] std::array<ehvota::state::SavedComponentBinding, 4> copySavedComponentBindings() const;

    [[nodiscard]] bool beginComponentCapture(
        double liveDurationSeconds,
        double referenceStartSeconds,
        double referenceDurationSeconds,
        const ehvota::state::AnalysisScope& scope);
    [[nodiscard]] ComponentCaptureBatch copyComponentCaptureBatch() const;
    [[nodiscard]] std::int64_t currentHostSample() const noexcept { return currentHostSample_.load(std::memory_order_acquire); }

    [[nodiscard]] bool stageRefEqProposal(
        const ehvota::analysis::SectionComparison& comparison,
        ehvota::communication::SharedInstanceId target,
        std::string evidenceId);
    [[nodiscard]] bool stageLatestRefEqProposal(ehvota::communication::SharedInstanceId target);
    [[nodiscard]] std::optional<ehvota::safety::Proposal> copyStagedProposal() const;
    void rejectStagedProposal();
    [[nodiscard]] bool approveAndSubmitStagedProposal();
    [[nodiscard]] bool submitApprovedProposal(
        const ehvota::safety::Proposal& proposal,
        ehvota::communication::SharedInstanceId target);

    [[nodiscard]] std::vector<ehvota::safety::ProposalHistoryEntry> copyProposalHistory() const;
    [[nodiscard]] bool undoLastAppliedProposal();

private:
    [[nodiscard]] bool startSectionAnalysisInternal(
        std::optional<ehvota::communication::SharedInstanceId> refEqTarget,
        double referenceStartSeconds,
        double referenceDurationSeconds,
        std::string sectionId);
    void timerCallback() override;
    void applyResolvedSectionLevelMatch();
    void recoverSavedComponentMappings(
        const std::array<ehvota::communication::SharedInstanceSnapshot,
            ehvota::communication::sharedLinkMaxInstances>& snapshots,
        std::size_t count);
    void reconcileStagedProposal(
        const std::array<ehvota::communication::SharedInstanceSnapshot,
            ehvota::communication::sharedLinkMaxInstances>& snapshots,
        std::size_t count);
    void restoreReferenceFromSavedPath();
    void stopAnalysisThread() noexcept;

    juce::AudioFormatManager formatManager_;
    std::unique_ptr<juce::AudioFormatReaderSource> referenceReaderSource_;
    juce::AudioTransportSource referenceTransport_;
    mutable juce::CriticalSection referenceLock_;
    juce::File referenceFile_;
    juce::String savedReferencePath_;
    bool referenceRestoreAttempted_ {};
    std::atomic<std::uint64_t> referenceGeneration_ { 1 };
    ehvota::audio::CaptureBuffer mixCapture_;

    std::atomic<float> livePeakDb_ { -100.0f };
    std::atomic<float> liveRmsDb_ { -100.0f };
    std::atomic<std::uint64_t> sampleCounter_ {};
    std::atomic<std::int64_t> currentHostSample_ { -1 };
    std::atomic<double> currentSampleRate_ { 44100.0 };
    std::atomic<float> referenceTrimDb_ { 0.0f };
    std::atomic<bool> referenceLoaded_ { false };
    std::atomic<bool> referenceAudition_ { false };
    std::atomic<bool> renderSafe_ { true };
    std::atomic<RefHubLevelMatchMode> levelMatchMode_ { RefHubLevelMatchMode::autoSection };
    std::atomic<bool> blindComparison_ { false };

    std::jthread analysisThread_;
    std::atomic<bool> analysisRunning_ { false };
    std::atomic<SectionAnalysisState> analysisState_ { SectionAnalysisState::idle };
    mutable juce::CriticalSection analysisLock_;
    std::optional<SectionAnalysisSnapshot> latestSectionAnalysis_;

    ehvota::plugins::RefLinkEndpoint refLink_ {
        ehvota::communication::SharedPluginKind::refHub,
        ehvota::communication::RefLinkRole::master,
        "RefHub"
    };
    mutable juce::CriticalSection linkCacheLock_;
    std::array<ehvota::communication::SharedInstanceSnapshot,
        ehvota::communication::sharedLinkMaxInstances> linkedInstances_ {};
    std::atomic<std::size_t> linkedCount_ {};
    ComponentCaptureCoordinator componentCoordinator_;
    mutable juce::CriticalSection savedMappingLock_;
    std::array<ehvota::state::SavedComponentBinding, 4> savedComponentBindings_;

    mutable juce::CriticalSection proposalLock_;
    std::optional<ehvota::safety::Proposal> stagedProposal_;
    ehvota::communication::SharedInstanceId stagedTarget_;
    ehvota::safety::ProposalHistory proposalHistory_ { 64 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefHubAudioProcessor)
};

} // namespace ehvota::refhub
