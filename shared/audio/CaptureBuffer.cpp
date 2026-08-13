#include "CaptureBuffer.h"

#include <algorithm>
#include <cmath>

namespace ehvota::audio {

void CaptureBuffer::prepare(const double sampleRate, const double maximumSeconds)
{
    reset();
    if (!(sampleRate > 0.0) || !(maximumSeconds > 0.0)) {
        left_.clear();
        right_.clear();
        sampleRate_ = 0.0;
        state_.store(CaptureState::unprepared, std::memory_order_release);
        return;
    }

    const auto maximumSamples = static_cast<std::size_t>(std::ceil(sampleRate * maximumSeconds));
    left_.assign(maximumSamples, 0.0f);
    right_.assign(maximumSamples, 0.0f);
    sampleRate_ = sampleRate;
    writeIndex_.store(0, std::memory_order_relaxed);
    targetSamples_.store(0, std::memory_order_relaxed);
    state_.store(CaptureState::idle, std::memory_order_release);
}

void CaptureBuffer::reset() noexcept
{
    const auto current = state_.load(std::memory_order_acquire);
    if (current == CaptureState::capturing || current == CaptureState::armed)
        state_.store(CaptureState::cancelled, std::memory_order_release);
    writeIndex_.store(0, std::memory_order_relaxed);
    targetSamples_.store(0, std::memory_order_relaxed);
    if (!left_.empty() && sampleRate_ > 0.0)
        state_.store(CaptureState::idle, std::memory_order_release);
}

bool CaptureBuffer::arm(const double durationSeconds) noexcept
{
    if (left_.empty() || sampleRate_ <= 0.0 || !(durationSeconds > 0.0))
        return false;

    const auto requested = static_cast<std::size_t>(std::llround(durationSeconds * sampleRate_));
    if (requested == 0 || requested > left_.size())
        return false;

    const auto current = state_.load(std::memory_order_acquire);
    if (current == CaptureState::capturing || current == CaptureState::armed)
        return false;

    writeIndex_.store(0, std::memory_order_relaxed);
    targetSamples_.store(requested, std::memory_order_relaxed);
    generation_.fetch_add(1, std::memory_order_acq_rel);
    state_.store(CaptureState::armed, std::memory_order_release);
    return true;
}

void CaptureBuffer::cancel() noexcept
{
    const auto current = state_.load(std::memory_order_acquire);
    if (current == CaptureState::armed || current == CaptureState::capturing)
        state_.store(CaptureState::cancelled, std::memory_order_release);
}

void CaptureBuffer::push(std::span<const float> left, std::span<const float> right) noexcept
{
    auto current = state_.load(std::memory_order_acquire);
    if (current == CaptureState::armed) {
        auto expected = CaptureState::armed;
        if (state_.compare_exchange_strong(expected, CaptureState::capturing, std::memory_order_acq_rel))
            current = CaptureState::capturing;
        else
            current = expected;
    }
    if (current != CaptureState::capturing)
        return;

    const auto target = targetSamples_.load(std::memory_order_relaxed);
    auto write = writeIndex_.load(std::memory_order_relaxed);
    if (write >= target || target > left_.size()) {
        state_.store(CaptureState::complete, std::memory_order_release);
        return;
    }

    const auto availableInput = std::min(left.size(), right.size());
    const auto count = std::min(availableInput, target - write);
    if (count == 0)
        return;

    std::copy_n(left.data(), count, left_.data() + write);
    std::copy_n(right.data(), count, right_.data() + write);
    write += count;
    writeIndex_.store(write, std::memory_order_release);

    if (write >= target)
        state_.store(CaptureState::complete, std::memory_order_release);
}

double CaptureBuffer::progress() const noexcept
{
    const auto target = targetSamples_.load(std::memory_order_acquire);
    if (target == 0)
        return 0.0;
    const auto written = std::min(writeIndex_.load(std::memory_order_acquire), target);
    return static_cast<double>(written) / static_cast<double>(target);
}

CaptureView CaptureBuffer::completedView() const noexcept
{
    if (state_.load(std::memory_order_acquire) != CaptureState::complete)
        return {};

    const auto count = std::min(writeIndex_.load(std::memory_order_acquire), targetSamples_.load(std::memory_order_acquire));
    return {
        std::span<const float>(left_.data(), count),
        std::span<const float>(right_.data(), count),
        sampleRate_,
        generation_.load(std::memory_order_acquire)
    };
}

} // namespace ehvota::audio
