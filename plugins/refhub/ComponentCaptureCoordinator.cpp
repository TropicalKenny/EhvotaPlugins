#include "ComponentCaptureCoordinator.h"

#include "ReferenceSectionReader.h"
#include "shared/analysis/CapturedSectionAnalysis.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace ehvota::refhub {
namespace {
constexpr std::array<ehvota::state::ReferenceRole, 4> componentRoles {
    ehvota::state::ReferenceRole::drums,
    ehvota::state::ReferenceRole::bass,
    ehvota::state::ReferenceRole::vocals,
    ehvota::state::ReferenceRole::music
};

const ehvota::communication::SharedInstanceSnapshot* findLinked(
    const std::array<ehvota::communication::SharedInstanceSnapshot,
        ehvota::communication::sharedLinkMaxInstances>& linked,
    const std::size_t count,
    const ehvota::communication::SharedInstanceId target) noexcept
{
    for (std::size_t i = 0; i < std::min(count, linked.size()); ++i) {
        if (linked[i].id.slot == target.slot && linked[i].id.generation == target.generation)
            return &linked[i];
    }
    return nullptr;
}
}

ComponentCaptureCoordinator::ComponentCaptureCoordinator()
{
    for (std::size_t i = 0; i < mappings_.size(); ++i)
        mappings_[i].role = componentRoles[i];
    for (std::size_t i = 0; i < batch_.roles.size(); ++i)
        batch_.roles[i].role = componentRoles[i];
}

ComponentCaptureCoordinator::~ComponentCaptureCoordinator()
{
    stopWorker();
}

void ComponentCaptureCoordinator::setReferenceFolder(const ReferenceFolderLoadResult& reference)
{
    const juce::ScopedLock lock(lock_);
    reference_ = reference;
}

std::size_t ComponentCaptureCoordinator::roleIndex(const ehvota::state::ReferenceRole role) noexcept
{
    for (std::size_t i = 0; i < componentRoles.size(); ++i)
        if (componentRoles[i] == role) return i;
    return componentRoles.size();
}

ehvota::state::ReferenceRole ComponentCaptureCoordinator::roleAt(const std::size_t index) noexcept
{
    return index < componentRoles.size() ? componentRoles[index] : ehvota::state::ReferenceRole::other;
}

bool ComponentCaptureCoordinator::setMapping(
    const ehvota::state::ReferenceRole role,
    const ehvota::communication::SharedInstanceId refTap)
{
    const auto index = roleIndex(role);
    if (index >= mappings_.size() || !refTap.valid())
        return false;
    const juce::ScopedLock lock(lock_);
    mappings_[index] = { role, refTap, true };
    return true;
}

void ComponentCaptureCoordinator::clearMapping(const ehvota::state::ReferenceRole role)
{
    const auto index = roleIndex(role);
    if (index >= mappings_.size())
        return;
    const juce::ScopedLock lock(lock_);
    mappings_[index] = { role, {}, false };
}

std::array<RuntimeComponentMapping, 4> ComponentCaptureCoordinator::copyMappings() const
{
    const juce::ScopedLock lock(lock_);
    return mappings_;
}

bool ComponentCaptureCoordinator::begin(
    ehvota::plugins::RefLinkEndpoint& endpoint,
    const std::array<ehvota::communication::SharedInstanceSnapshot,
        ehvota::communication::sharedLinkMaxInstances>& linked,
    const std::size_t linkedCount,
    const std::int64_t currentHostSample,
    const double sampleRate,
    const double liveDurationSeconds,
    const double referenceStartSeconds,
    const double referenceDurationSeconds,
    const ehvota::state::AnalysisScope& scope)
{
    using namespace ehvota::communication;
    if (busy() || currentHostSample < 0 || !(sampleRate >= 8000.0)
        || !std::isfinite(sampleRate) || !(liveDurationSeconds > 0.0)
        || liveDurationSeconds > 60.0 || !(referenceStartSeconds >= 0.0)
        || !(referenceDurationSeconds > 0.0) || referenceDurationSeconds > 60.0
        || !std::isfinite(referenceStartSeconds) || !std::isfinite(referenceDurationSeconds))
        return false;

    const auto duration64 = std::llround(liveDurationSeconds * sampleRate);
    if (duration64 <= 0 || duration64 > sharedCaptureMaximumSamples)
        return false;

    ReferenceFolderLoadResult reference;
    std::array<RuntimeComponentMapping, 4> mappings;
    {
        const juce::ScopedLock lock(lock_);
        reference = reference_;
        mappings = mappings_;
    }
    if (!reference.validation.valid)
        return false;

    auto batchId = nextBatchId_.fetch_add(1, std::memory_order_relaxed);
    if (batchId == 0)
        batchId = nextBatchId_.fetch_add(1, std::memory_order_relaxed);

    ComponentCaptureBatch candidate;
    candidate.batchId = batchId;
    candidate.state = ComponentCaptureBatchState::scheduled;
    candidate.sampleRate = sampleRate;
    candidate.durationSamples = static_cast<std::uint32_t>(duration64);
    candidate.referenceStartSeconds = referenceStartSeconds;
    candidate.referenceDurationSeconds = referenceDurationSeconds;
    // One second gives every 20 Hz RefTap control timer ample time to arm before the common start.
    candidate.startHostSample = currentHostSample + static_cast<std::int64_t>(std::llround(sampleRate));

    std::size_t submitted = 0;
    for (std::size_t i = 0; i < candidate.roles.size(); ++i) {
        auto& roleResult = candidate.roles[i];
        roleResult.role = roleAt(i);
        roleResult.requested = scope.includes(roleResult.role);
        if (!roleResult.requested) {
            roleResult.message = "Excluded from this analysis scope.";
            continue;
        }

        const auto referenceFile = referenceFileForRole(reference, roleResult.role);
        const auto referenceSource = referenceSourceForRole(reference, roleResult.role);
        roleResult.referenceAvailable = referenceFile.has_value() && referenceSource.has_value();
        roleResult.alignmentVerified = referenceSource.has_value() && referenceSource->alignedToFullMix;
        if (!roleResult.referenceAvailable) {
            roleResult.message = "Requested reference stem is unavailable.";
            continue;
        }

        const auto mapping = mappings[i];
        if (!mapping.enabled || !mapping.refTap.valid()) {
            roleResult.message = "No RefTap is mapped to this component.";
            continue;
        }
        const auto* snapshot = findLinked(linked, linkedCount, mapping.refTap);
        if (!snapshot || snapshot->pluginKind != SharedPluginKind::refTap || snapshot->heartbeatAgeMs > 5000) {
            roleResult.message = "Mapped RefTap is disconnected or stale.";
            continue;
        }

        roleResult.refTap = mapping.refTap;
        roleResult.captureId = batchId * 16ULL + static_cast<std::uint64_t>(i + 1);
        if (roleResult.captureId == 0)
            roleResult.captureId = static_cast<std::uint64_t>(i + 1);

        SharedCaptureRequest request;
        request.captureId = roleResult.captureId;
        request.targetGeneration = mapping.refTap.generation;
        request.startHostSample = candidate.startHostSample;
        request.durationSamples = candidate.durationSamples;
        if (!endpoint.submitCaptureRequest(mapping.refTap, request)) {
            roleResult.message = "RefTap rejected the scheduled capture request.";
            continue;
        }

        roleResult.submitted = true;
        roleResult.transportState = SharedCaptureState::pending;
        roleResult.message = roleResult.alignmentVerified
            ? "Capture scheduled."
            : "Capture scheduled; reference-stem alignment is unverified, so results remain diagnostic-only.";
        ++submitted;
    }

    if (submitted == 0) {
        candidate.state = ComponentCaptureBatchState::error;
        const juce::ScopedLock lock(lock_);
        batch_ = std::move(candidate);
        return false;
    }

    const juce::ScopedLock lock(lock_);
    batch_ = std::move(candidate);
    return true;
}

ehvota::analysis::CompactSectionAnalysis ComponentCaptureCoordinator::fromShared(
    const ehvota::communication::SharedCapturedAnalysis& analysis) noexcept
{
    ehvota::analysis::CompactSectionAnalysis result;
    if (!ehvota::communication::validateSharedCapturedAnalysis(analysis))
        return result;
    result.valid = true;
    result.integratedLufs = analysis.integratedLufs;
    result.peakDb = analysis.peakDb;
    result.rmsDb = analysis.rmsDb;
    result.crestDb = analysis.crestDb;
    result.correlation = analysis.correlation;
    result.sideToMidDb = analysis.sideToMidDb;
    for (std::size_t i = 0; i < result.bandPowerDb.size(); ++i)
        result.bandPowerDb[i] = analysis.bandPowerDb[i];
    return result;
}

void ComponentCaptureCoordinator::reconcile(
    const std::array<ehvota::communication::SharedInstanceSnapshot,
        ehvota::communication::sharedLinkMaxInstances>& linked,
    const std::size_t linkedCount)
{
    if (workerRunning_.load(std::memory_order_acquire))
        return;

    std::vector<ComparisonWork> work;
    bool anyInFlight = false;
    bool anySubmitted = false;
    {
        const juce::ScopedLock lock(lock_);
        if (batch_.state == ComponentCaptureBatchState::idle
            || batch_.state == ComponentCaptureBatchState::complete
            || batch_.state == ComponentCaptureBatchState::partial
            || batch_.state == ComponentCaptureBatchState::error
            || batch_.state == ComponentCaptureBatchState::analysing)
            return;

        for (std::size_t i = 0; i < batch_.roles.size(); ++i) {
            auto& role = batch_.roles[i];
            if (!role.submitted)
                continue;
            anySubmitted = true;
            const auto* snapshot = findLinked(linked, linkedCount, role.refTap);
            if (!snapshot) {
                role.transportState = ehvota::communication::SharedCaptureState::error;
                role.message = "Mapped RefTap disconnected during capture.";
                continue;
            }
            if (snapshot->captureRequest.captureId != role.captureId) {
                if (ehvota::communication::sharedCaptureStateInFlight(role.transportState))
                    anyInFlight = true;
                continue;
            }

            role.transportState = snapshot->captureRequest.state;
            if (ehvota::communication::sharedCaptureStateInFlight(role.transportState)) {
                anyInFlight = true;
                if (role.transportState == ehvota::communication::SharedCaptureState::capturing)
                    batch_.state = ComponentCaptureBatchState::capturing;
                continue;
            }

            if (role.transportState == ehvota::communication::SharedCaptureState::complete
                && snapshot->capturedAnalysis.captureId == role.captureId) {
                role.live = fromShared(snapshot->capturedAnalysis);
                if (!role.live.valid) {
                    role.message = "RefTap capture summary failed validation.";
                    continue;
                }
                const auto file = referenceFileForRole(reference_, role.role);
                if (!file) {
                    role.message = "Reference stem disappeared before comparison.";
                    continue;
                }
                work.push_back({
                    batch_.batchId,
                    i,
                    *file,
                    role.alignmentVerified,
                    role.live,
                    batch_.sampleRate,
                    batch_.referenceStartSeconds,
                    batch_.referenceDurationSeconds
                });
            } else if (role.transportState == ehvota::communication::SharedCaptureState::rejected
                || role.transportState == ehvota::communication::SharedCaptureState::error) {
                role.message = "RefTap capture did not complete (code "
                    + std::to_string(snapshot->captureRequest.resultCode) + ").";
            }
        }

        if (anyInFlight)
            return;
        if (!anySubmitted) {
            batch_.state = ComponentCaptureBatchState::error;
            return;
        }
        if (work.empty()) {
            batch_.state = ComponentCaptureBatchState::error;
            return;
        }
        batch_.state = ComponentCaptureBatchState::analysing;
    }

    launchReferenceComparisons(std::move(work));
}

void ComponentCaptureCoordinator::launchReferenceComparisons(std::vector<ComparisonWork> work)
{
    if (worker_.joinable())
        worker_.join();
    workerRunning_.store(true, std::memory_order_release);

    worker_ = std::jthread([this, work = std::move(work)](const std::stop_token stopToken) mutable {
        std::size_t ready = 0;
        const auto expectedBatch = work.empty() ? 0 : work.front().batchId;
        for (auto& item : work) {
            if (stopToken.stop_requested())
                break;
            const auto targetSamples64 = std::llround(item.referenceDurationSeconds * item.sampleRate);
            if (targetSamples64 <= 0
                || static_cast<unsigned long long>(targetSamples64) > std::numeric_limits<std::size_t>::max())
                continue;
            const auto referenceAudio = readReferenceSection(
                item.referenceFile,
                item.referenceStartSeconds,
                item.sampleRate,
                static_cast<std::size_t>(targetSamples64));
            if (!referenceAudio.valid)
                continue;

            auto captured = ehvota::analysis::analyzeCapturedSection(
                "component-reference",
                referenceAudio.left,
                referenceAudio.right,
                item.sampleRate);
            const auto compactReference = ehvota::analysis::compactCapturedSection(captured);
            const auto comparison = ehvota::analysis::compareCompactSections(item.live, compactReference);
            if (!compactReference.valid || !comparison.valid)
                continue;

            const juce::ScopedLock lock(lock_);
            if (batch_.batchId != expectedBatch || item.roleIndex >= batch_.roles.size())
                continue;
            auto& role = batch_.roles[item.roleIndex];
            role.reference = compactReference;
            role.comparison = comparison;
            role.comparisonReady = true;
            role.message = item.alignmentVerified
                ? "Component comparison complete."
                : "Component comparison complete; alignment remains unverified, so this is diagnostic-only.";
            ++ready;
        }

        if (!stopToken.stop_requested()) {
            const juce::ScopedLock lock(lock_);
            if (batch_.batchId == expectedBatch) {
                std::size_t requested = 0;
                std::size_t compared = 0;
                bool hadUnavailable = false;
                for (const auto& role : batch_.roles) {
                    if (!role.requested) continue;
                    ++requested;
                    if (role.comparisonReady) ++compared;
                    else hadUnavailable = true;
                }
                if (compared == 0)
                    batch_.state = ComponentCaptureBatchState::error;
                else if (!hadUnavailable && compared == requested)
                    batch_.state = ComponentCaptureBatchState::complete;
                else
                    batch_.state = ComponentCaptureBatchState::partial;
            }
        }
        workerRunning_.store(false, std::memory_order_release);
    });
}

ComponentCaptureBatch ComponentCaptureCoordinator::copyBatch() const
{
    const juce::ScopedLock lock(lock_);
    return batch_;
}

bool ComponentCaptureCoordinator::busy() const noexcept
{
    if (workerRunning_.load(std::memory_order_acquire))
        return true;
    const juce::ScopedLock lock(lock_);
    return batch_.state == ComponentCaptureBatchState::scheduled
        || batch_.state == ComponentCaptureBatchState::capturing
        || batch_.state == ComponentCaptureBatchState::analysing;
}

void ComponentCaptureCoordinator::stopWorker() noexcept
{
    if (worker_.joinable()) {
        worker_.request_stop();
        worker_.join();
    }
    workerRunning_.store(false, std::memory_order_release);
}

} // namespace ehvota::refhub
