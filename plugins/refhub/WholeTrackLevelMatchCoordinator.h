#pragma once

#include "ReferenceWholeTrackLoudness.h"
#include "shared/analysis/LevelMatch.h"
#include "shared/analysis/StreamingLoudness1770.h"

#include <juce_core/juce_core.h>

#include <atomic>
#include <cstdint>
#include <span>
#include <string>
#include <thread>

namespace ehvota::refhub {

enum class WholeTrackLevelMatchState : std::uint8_t {
    idle,
    measuring,
    finalizing,
    partial,
    ready,
    stale,
    error
};

struct WholeTrackLevelMatchSnapshot {
    WholeTrackLevelMatchState state { WholeTrackLevelMatchState::idle };
    std::uint64_t generation {};
    std::uint64_t referenceGeneration {};
    bool mixComplete {};
    bool referenceComplete {};
    ehvota::analysis::LoudnessResult mix;
    ehvota::analysis::LoudnessResult reference;
    ehvota::analysis::LevelMatchDecision levelMatch;
    std::string message;
};

// Owns the whole-track evidence lifecycle. The audio callback only enters process(),
// which performs no allocation, locking, file I/O, thread creation, or waiting.
class WholeTrackLevelMatchCoordinator {
public:
    WholeTrackLevelMatchCoordinator() = default;
    ~WholeTrackLevelMatchCoordinator();

    WholeTrackLevelMatchCoordinator(const WholeTrackLevelMatchCoordinator&) = delete;
    WholeTrackLevelMatchCoordinator& operator=(const WholeTrackLevelMatchCoordinator&) = delete;

    [[nodiscard]] bool prepare(double sampleRate, double maximumDurationSeconds = 14400.0);
    [[nodiscard]] bool begin(const juce::File& referenceFile, std::uint64_t referenceGeneration);

    void process(
        std::span<const float> left,
        std::span<const float> right) noexcept;

    void requestFinishMix() noexcept;
    void poll();
    void cancel();
    void invalidate(std::string message);

    [[nodiscard]] WholeTrackLevelMatchSnapshot snapshot() const;
    [[nodiscard]] bool ready(std::uint64_t currentReferenceGeneration) const;
    [[nodiscard]] bool mixActive() const noexcept { return mixActive_.load(std::memory_order_acquire); }

private:
    void stopReferenceThread() noexcept;
    void resolveLocked();
    void setErrorLocked(std::string message);
    void waitForAudioWriterToExit() const noexcept;

    ehvota::analysis::StreamingLoudness1770 mixMeter_;
    double preparedSampleRate_ {};
    double maximumDurationSeconds_ { 14400.0 };
    bool prepared_ {};

    std::atomic<bool> mixActive_ { false };
    std::atomic<bool> mixFinalizePending_ { false };
    std::atomic<bool> mixOverflowed_ { false };
    std::atomic<std::uint32_t> audioWriters_ {};

    std::jthread referenceThread_;
    std::atomic<std::uint64_t> nextGeneration_ { 1 };

    mutable juce::CriticalSection lock_;
    WholeTrackLevelMatchSnapshot snapshot_;
};

} // namespace ehvota::refhub
