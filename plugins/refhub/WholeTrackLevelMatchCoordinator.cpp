#include "WholeTrackLevelMatchCoordinator.h"

#include <cmath>
#include <utility>

namespace ehvota::refhub {

WholeTrackLevelMatchCoordinator::~WholeTrackLevelMatchCoordinator()
{
    cancel();
}

void WholeTrackLevelMatchCoordinator::waitForAudioWriterToExit() const noexcept
{
    // Message/worker-thread lifecycle code may wait; the audio callback never waits here.
    while (audioWriters_.load(std::memory_order_acquire) != 0)
        std::this_thread::yield();
}

void WholeTrackLevelMatchCoordinator::stopReferenceThread() noexcept
{
    if (!referenceThread_.joinable())
        return;
    referenceThread_.request_stop();
    referenceThread_.join();
}

bool WholeTrackLevelMatchCoordinator::prepare(
    const double sampleRate,
    const double maximumDurationSeconds)
{
    cancel();
    prepared_ = mixMeter_.prepare(sampleRate, maximumDurationSeconds);
    preparedSampleRate_ = prepared_ ? sampleRate : 0.0;
    maximumDurationSeconds_ = maximumDurationSeconds;
    return prepared_;
}

bool WholeTrackLevelMatchCoordinator::begin(
    const juce::File& referenceFile,
    const std::uint64_t referenceGeneration)
{
    if (!prepared_ || !(preparedSampleRate_ >= 8000.0) || !referenceFile.existsAsFile()
        || referenceGeneration == 0)
        return false;

    cancel();
    if (!mixMeter_.begin())
        return false;

    auto generation = nextGeneration_.fetch_add(1, std::memory_order_relaxed);
    if (generation == 0)
        generation = nextGeneration_.fetch_add(1, std::memory_order_relaxed);

    {
        const juce::ScopedLock scoped(lock_);
        snapshot_ = {};
        snapshot_.state = WholeTrackLevelMatchState::measuring;
        snapshot_.generation = generation;
        snapshot_.referenceGeneration = referenceGeneration;
        snapshot_.message = "Play the complete mix once, then finish the mix pass. Reference loudness is measured independently in the background.";
    }

    mixOverflowed_.store(false, std::memory_order_release);
    mixFinalizePending_.store(false, std::memory_order_release);
    mixActive_.store(true, std::memory_order_release);

    referenceThread_ = std::jthread(
        [this, referenceFile, generation, referenceGeneration](const std::stop_token stopToken) {
            auto measured = measureReferenceWholeTrackLoudness(referenceFile, stopToken);
            if (stopToken.stop_requested())
                return;

            const juce::ScopedLock scoped(lock_);
            if (snapshot_.generation != generation
                || snapshot_.referenceGeneration != referenceGeneration)
                return;

            snapshot_.referenceComplete = true;
            if (measured.valid) {
                snapshot_.reference = measured.loudness;
            } else {
                snapshot_.reference = {};
                snapshot_.message = measured.error.empty()
                    ? "Reference whole-track loudness measurement failed."
                    : measured.error;
            }
            resolveLocked();
        });

    return true;
}

void WholeTrackLevelMatchCoordinator::process(
    const std::span<const float> left,
    const std::span<const float> right) noexcept
{
    if (!mixActive_.load(std::memory_order_acquire))
        return;

    audioWriters_.fetch_add(1, std::memory_order_acq_rel);
    if (mixActive_.load(std::memory_order_acquire)) {
        mixMeter_.processStereo(left, right);
        if (mixMeter_.state() == ehvota::analysis::StreamingLoudnessState::capacityExceeded) {
            mixOverflowed_.store(true, std::memory_order_release);
            mixActive_.store(false, std::memory_order_release);
            mixFinalizePending_.store(true, std::memory_order_release);
        }
    }
    audioWriters_.fetch_sub(1, std::memory_order_release);
}

void WholeTrackLevelMatchCoordinator::requestFinishMix() noexcept
{
    const auto wasActive = mixActive_.exchange(false, std::memory_order_acq_rel);
    if (!wasActive && !mixOverflowed_.load(std::memory_order_acquire))
        return;

    mixFinalizePending_.store(true, std::memory_order_release);
    const juce::ScopedLock scoped(lock_);
    if (snapshot_.state == WholeTrackLevelMatchState::measuring
        || snapshot_.state == WholeTrackLevelMatchState::partial)
        snapshot_.state = WholeTrackLevelMatchState::finalizing;
}

void WholeTrackLevelMatchCoordinator::poll()
{
    if (!mixFinalizePending_.load(std::memory_order_acquire)
        || mixActive_.load(std::memory_order_acquire)
        || audioWriters_.load(std::memory_order_acquire) != 0)
        return;

    if (!mixFinalizePending_.exchange(false, std::memory_order_acq_rel))
        return;

    const auto measured = mixMeter_.finish();
    const juce::ScopedLock scoped(lock_);
    snapshot_.mixComplete = true;
    if (mixOverflowed_.exchange(false, std::memory_order_acq_rel)
        || measured.state == ehvota::analysis::StreamingLoudnessState::capacityExceeded) {
        snapshot_.mix = {};
        setErrorLocked("Mix whole-track pass exceeded the prepared measurement capacity.");
        return;
    }

    snapshot_.mix = measured.loudness;
    if (!snapshot_.mix.valid) {
        setErrorLocked("Mix whole-track pass did not contain enough valid programme material for integrated loudness.");
        return;
    }
    resolveLocked();
}

void WholeTrackLevelMatchCoordinator::resolveLocked()
{
    if (snapshot_.mixComplete && snapshot_.referenceComplete) {
        if (!snapshot_.mix.valid || !snapshot_.reference.valid) {
            if (snapshot_.message.empty())
                snapshot_.message = "Whole-track evidence is incomplete or invalid.";
            snapshot_.state = WholeTrackLevelMatchState::error;
            snapshot_.levelMatch = {};
            return;
        }

        snapshot_.levelMatch = ehvota::analysis::calculateReferenceMonitorTrim(
            snapshot_.mix, snapshot_.reference);
        if (!snapshot_.levelMatch.valid) {
            snapshot_.state = WholeTrackLevelMatchState::error;
            snapshot_.message = "Whole-track loudness values could not produce a safe monitoring trim.";
            return;
        }

        snapshot_.state = WholeTrackLevelMatchState::ready;
        snapshot_.message = "Whole-track evidence is stable. The calculated trim is monitoring-only and does not modify either source.";
        return;
    }

    if (snapshot_.mixComplete || snapshot_.referenceComplete) {
        snapshot_.state = WholeTrackLevelMatchState::partial;
        if (snapshot_.mixComplete)
            snapshot_.message = "Mix whole-track pass is complete; reference measurement is still running.";
        else
            snapshot_.message = "Reference whole-track measurement is complete; finish the mix pass when the full song has played.";
        return;
    }

    snapshot_.state = WholeTrackLevelMatchState::measuring;
}

void WholeTrackLevelMatchCoordinator::setErrorLocked(std::string message)
{
    snapshot_.state = WholeTrackLevelMatchState::error;
    snapshot_.levelMatch = {};
    snapshot_.message = std::move(message);
}

void WholeTrackLevelMatchCoordinator::cancel()
{
    mixActive_.store(false, std::memory_order_release);
    waitForAudioWriterToExit();
    mixFinalizePending_.store(false, std::memory_order_release);
    mixOverflowed_.store(false, std::memory_order_release);
    stopReferenceThread();

    const juce::ScopedLock scoped(lock_);
    snapshot_ = {};
    snapshot_.state = WholeTrackLevelMatchState::idle;
}

void WholeTrackLevelMatchCoordinator::invalidate(std::string message)
{
    mixActive_.store(false, std::memory_order_release);
    waitForAudioWriterToExit();
    mixFinalizePending_.store(false, std::memory_order_release);
    mixOverflowed_.store(false, std::memory_order_release);
    stopReferenceThread();

    const juce::ScopedLock scoped(lock_);
    const auto previousGeneration = snapshot_.generation;
    snapshot_ = {};
    snapshot_.generation = previousGeneration;
    snapshot_.state = WholeTrackLevelMatchState::stale;
    snapshot_.message = std::move(message);
}

WholeTrackLevelMatchSnapshot WholeTrackLevelMatchCoordinator::snapshot() const
{
    const juce::ScopedLock scoped(lock_);
    return snapshot_;
}

bool WholeTrackLevelMatchCoordinator::ready(
    const std::uint64_t currentReferenceGeneration) const
{
    const juce::ScopedLock scoped(lock_);
    return snapshot_.state == WholeTrackLevelMatchState::ready
        && snapshot_.levelMatch.valid
        && snapshot_.referenceGeneration == currentReferenceGeneration;
}

} // namespace ehvota::refhub
