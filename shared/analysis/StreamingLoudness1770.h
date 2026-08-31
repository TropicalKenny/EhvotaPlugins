#pragma once

#include "Loudness1770.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ehvota::analysis {

enum class StreamingLoudnessState : std::uint8_t {
    unprepared,
    idle,
    measuring,
    complete,
    capacityExceeded
};

struct StreamingLoudnessResult {
    LoudnessResult loudness;
    StreamingLoudnessState state { StreamingLoudnessState::unprepared };
    std::uint64_t samplesProcessed {};
    std::uint64_t maximumSamples {};
};

// Allocation-free while processing. prepare() allocates the fixed ring buffer and the
// maximum number of 100 ms loudness-block entries before the audio thread starts.
class StreamingLoudness1770 {
public:
    StreamingLoudness1770() = default;

    [[nodiscard]] bool prepare(double sampleRate, double maximumDurationSeconds = 3600.0);
    void reset() noexcept;
    [[nodiscard]] bool begin() noexcept;

    void processStereo(
        std::span<const float> left,
        std::span<const float> right) noexcept;

    [[nodiscard]] StreamingLoudnessResult finish() noexcept;

    [[nodiscard]] StreamingLoudnessState state() const noexcept { return state_; }
    [[nodiscard]] std::uint64_t samplesProcessed() const noexcept { return samplesProcessed_; }
    [[nodiscard]] double sampleRate() const noexcept { return sampleRate_; }

private:
    struct BiquadState {
        double b0 { 1.0 };
        double b1 {};
        double b2 {};
        double a1 {};
        double a2 {};
        double z1 {};
        double z2 {};

        [[nodiscard]] double process(double input) noexcept;
        void clear() noexcept { z1 = 0.0; z2 = 0.0; }
    };

    struct ChannelKWeighting {
        BiquadState shelf;
        BiquadState highPass;
        [[nodiscard]] double process(double sample) noexcept;
        void clear() noexcept { shelf.clear(); highPass.clear(); }
    };

    [[nodiscard]] static BiquadState makeHighShelf(double sampleRate) noexcept;
    [[nodiscard]] static BiquadState makeHighPass(double sampleRate) noexcept;
    [[nodiscard]] LoudnessResult calculateResult() const noexcept;

    double sampleRate_ {};
    std::size_t blockSize_ {};
    std::size_t hopSize_ {};
    std::uint64_t maximumSamples_ {};

    std::vector<double> energyRing_;
    std::vector<double> blockEnergy_;
    std::size_t ringIndex_ {};
    std::size_t blockCount_ {};
    double runningEnergy_ {};
    std::uint64_t samplesProcessed_ {};

    ChannelKWeighting leftFilter_;
    ChannelKWeighting rightFilter_;
    StreamingLoudnessState state_ { StreamingLoudnessState::unprepared };
};

} // namespace ehvota::analysis
