#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ehvota::audio {

enum class CaptureState : std::uint8_t {
    unprepared,
    idle,
    armed,
    capturing,
    complete,
    cancelled
};

struct CaptureView {
    std::span<const float> left;
    std::span<const float> right;
    double sampleRate {};
    std::uint64_t generation {};
};

class CaptureBuffer {
public:
    // Call off the audio thread. Allocates the maximum storage once.
    void prepare(double sampleRate, double maximumSeconds);
    void reset() noexcept;

    // Call off the audio thread. Returns false when duration is invalid or exceeds capacity.
    [[nodiscard]] bool arm(double durationSeconds) noexcept;
    void cancel() noexcept;

    // Audio-thread safe: bounded copies into preallocated storage; no locks or allocation.
    void push(std::span<const float> left, std::span<const float> right) noexcept;

    [[nodiscard]] CaptureState state() const noexcept { return state_.load(std::memory_order_acquire); }
    [[nodiscard]] double progress() const noexcept;
    [[nodiscard]] std::size_t capturedSamples() const noexcept { return writeIndex_.load(std::memory_order_acquire); }
    [[nodiscard]] std::size_t targetSamples() const noexcept { return targetSamples_.load(std::memory_order_acquire); }
    [[nodiscard]] std::uint64_t generation() const noexcept { return generation_.load(std::memory_order_acquire); }

    // Read only after state()==complete. The returned view remains valid until prepare/reset/arm.
    [[nodiscard]] CaptureView completedView() const noexcept;

private:
    std::vector<float> left_;
    std::vector<float> right_;
    double sampleRate_ {};
    std::atomic<CaptureState> state_ { CaptureState::unprepared };
    std::atomic<std::size_t> writeIndex_ {};
    std::atomic<std::size_t> targetSamples_ {};
    std::atomic<std::uint64_t> generation_ {};
};

} // namespace ehvota::audio
