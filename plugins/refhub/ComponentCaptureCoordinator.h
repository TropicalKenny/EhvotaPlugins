#pragma once

#include "ReferenceFolderLoader.h"
#include "plugins/common/RefLinkEndpoint.h"
#include "shared/analysis/CompactAnalysis.h"
#include "shared/state/AnalysisScope.h"

#include <juce_core/juce_core.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace ehvota::refhub {

enum class ComponentCaptureBatchState : std::uint8_t {
    idle,
    scheduled,
    capturing,
    analysing,
    complete,
    partial,
    error
};

struct RuntimeComponentMapping {
    ehvota::state::ReferenceRole role { ehvota::state::ReferenceRole::other };
    ehvota::communication::SharedInstanceId refTap;
    bool enabled {};
};

struct ComponentRoleCaptureResult {
    ehvota::state::ReferenceRole role { ehvota::state::ReferenceRole::other };
    bool requested {};
    bool submitted {};
    bool referenceAvailable {};
    bool alignmentVerified {};
    ehvota::communication::SharedInstanceId refTap;
    std::uint64_t captureId {};
    ehvota::communication::SharedCaptureState transportState { ehvota::communication::SharedCaptureState::empty };
    bool comparisonReady {};
    ehvota::analysis::CompactSectionAnalysis live;
    ehvota::analysis::CompactSectionAnalysis reference;
    ehvota::analysis::SectionComparison comparison;
    std::string message;
};

struct ComponentCaptureBatch {
    std::uint64_t batchId {};
    ComponentCaptureBatchState state { ComponentCaptureBatchState::idle };
    std::int64_t startHostSample { -1 };
    std::uint32_t durationSamples {};
    double sampleRate {};
    double referenceStartSeconds {};
    double referenceDurationSeconds {};
    std::array<ComponentRoleCaptureResult, 4> roles;
};

class ComponentCaptureCoordinator {
public:
    ComponentCaptureCoordinator();
    ~ComponentCaptureCoordinator();

    void setReferenceFolder(const ReferenceFolderLoadResult& reference);
    [[nodiscard]] bool setMapping(
        ehvota::state::ReferenceRole role,
        ehvota::communication::SharedInstanceId refTap);
    void clearMapping(ehvota::state::ReferenceRole role);
    [[nodiscard]] std::array<RuntimeComponentMapping, 4> copyMappings() const;

    // Schedules all usable requested roles to the same future host sample.
    [[nodiscard]] bool begin(
        ehvota::plugins::RefLinkEndpoint& endpoint,
        const std::array<ehvota::communication::SharedInstanceSnapshot,
            ehvota::communication::sharedLinkMaxInstances>& linked,
        std::size_t linkedCount,
        std::int64_t currentHostSample,
        double sampleRate,
        double liveDurationSeconds,
        double referenceStartSeconds,
        double referenceDurationSeconds,
        const ehvota::state::AnalysisScope& scope);

    void reconcile(
        const std::array<ehvota::communication::SharedInstanceSnapshot,
            ehvota::communication::sharedLinkMaxInstances>& linked,
        std::size_t linkedCount);

    [[nodiscard]] ComponentCaptureBatch copyBatch() const;
    [[nodiscard]] bool busy() const noexcept;

private:
    struct ComparisonWork {
        std::uint64_t batchId {};
        std::size_t roleIndex {};
        juce::File referenceFile;
        bool alignmentVerified {};
        ehvota::analysis::CompactSectionAnalysis live;
        double sampleRate {};
        double referenceStartSeconds {};
        double referenceDurationSeconds {};
    };

    [[nodiscard]] static std::size_t roleIndex(ehvota::state::ReferenceRole role) noexcept;
    [[nodiscard]] static ehvota::state::ReferenceRole roleAt(std::size_t index) noexcept;
    [[nodiscard]] static ehvota::analysis::CompactSectionAnalysis fromShared(
        const ehvota::communication::SharedCapturedAnalysis& analysis) noexcept;
    void launchReferenceComparisons(std::vector<ComparisonWork> work);
    void stopWorker() noexcept;

    mutable juce::CriticalSection lock_;
    ReferenceFolderLoadResult reference_;
    std::array<RuntimeComponentMapping, 4> mappings_;
    ComponentCaptureBatch batch_;
    std::atomic<std::uint64_t> nextBatchId_ { 1 };
    std::jthread worker_;
    std::atomic<bool> workerRunning_ { false };
};

} // namespace ehvota::refhub
