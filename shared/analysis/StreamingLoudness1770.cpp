#include "StreamingLoudness1770.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace ehvota::analysis {
namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double loudnessOffset = -0.691;
constexpr double absoluteGateLufs = -70.0;

[[nodiscard]] double energyToLufs(const double meanSquare) noexcept
{
    if (!(meanSquare > 0.0) || !std::isfinite(meanSquare))
        return -100.0;
    return loudnessOffset + 10.0 * std::log10(meanSquare);
}

[[nodiscard]] double lufsToEnergy(const double lufs) noexcept
{
    return std::pow(10.0, (lufs - loudnessOffset) / 10.0);
}

} // namespace

double StreamingLoudness1770::BiquadState::process(const double input) noexcept
{
    const auto output = b0 * input + z1;
    z1 = b1 * input - a1 * output + z2;
    z2 = b2 * input - a2 * output;
    return output;
}

double StreamingLoudness1770::ChannelKWeighting::process(const double sample) noexcept
{
    return highPass.process(shelf.process(sample));
}

StreamingLoudness1770::BiquadState StreamingLoudness1770::makeHighShelf(const double sampleRate) noexcept
{
    constexpr double frequency = 1681.974450955533;
    constexpr double gainDb = 3.999843853973347;
    constexpr double q = 0.7071752369554196;
    const auto a = std::pow(10.0, gainDb / 40.0);
    const auto w0 = 2.0 * pi * frequency / sampleRate;
    const auto alpha = std::sin(w0) / (2.0 * q);
    const auto cosw = std::cos(w0);
    const auto sqrtA = std::sqrt(a);

    const auto b0 = a * ((a + 1.0) + (a - 1.0) * cosw + 2.0 * sqrtA * alpha);
    const auto b1 = -2.0 * a * ((a - 1.0) + (a + 1.0) * cosw);
    const auto b2 = a * ((a + 1.0) + (a - 1.0) * cosw - 2.0 * sqrtA * alpha);
    const auto a0 = (a + 1.0) - (a - 1.0) * cosw + 2.0 * sqrtA * alpha;
    const auto a1 = 2.0 * ((a - 1.0) - (a + 1.0) * cosw);
    const auto a2 = (a + 1.0) - (a - 1.0) * cosw - 2.0 * sqrtA * alpha;

    BiquadState result;
    result.b0 = b0 / a0;
    result.b1 = b1 / a0;
    result.b2 = b2 / a0;
    result.a1 = a1 / a0;
    result.a2 = a2 / a0;
    return result;
}

StreamingLoudness1770::BiquadState StreamingLoudness1770::makeHighPass(const double sampleRate) noexcept
{
    constexpr double frequency = 38.13547087602444;
    constexpr double q = 0.5003270373238773;
    const auto w0 = 2.0 * pi * frequency / sampleRate;
    const auto alpha = std::sin(w0) / (2.0 * q);
    const auto cosw = std::cos(w0);
    const auto b0 = (1.0 + cosw) / 2.0;
    const auto b1 = -(1.0 + cosw);
    const auto b2 = (1.0 + cosw) / 2.0;
    const auto a0 = 1.0 + alpha;
    const auto a1 = -2.0 * cosw;
    const auto a2 = 1.0 - alpha;

    BiquadState result;
    result.b0 = b0 / a0;
    result.b1 = b1 / a0;
    result.b2 = b2 / a0;
    result.a1 = a1 / a0;
    result.a2 = a2 / a0;
    return result;
}

bool StreamingLoudness1770::prepare(const double sampleRate, const double maximumDurationSeconds)
{
    if (!std::isfinite(sampleRate) || sampleRate < 8000.0 || sampleRate > 768000.0
        || !std::isfinite(maximumDurationSeconds) || maximumDurationSeconds < 0.5
        || maximumDurationSeconds > 14400.0)
        return false;

    const auto maximumSamplesDouble = std::ceil(sampleRate * maximumDurationSeconds);
    if (!(maximumSamplesDouble > 0.0)
        || maximumSamplesDouble > static_cast<double>(std::numeric_limits<std::uint64_t>::max()))
        return false;

    const auto blockSize = static_cast<std::size_t>(std::llround(sampleRate * 0.400));
    const auto hopSize = static_cast<std::size_t>(std::llround(sampleRate * 0.100));
    const auto maximumSamples = static_cast<std::uint64_t>(maximumSamplesDouble);
    if (blockSize == 0 || hopSize == 0 || maximumSamples < blockSize)
        return false;

    const auto maximumBlocks64 = 1ULL + (maximumSamples - blockSize) / hopSize;
    if (maximumBlocks64 > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()))
        return false;

    try {
        energyRing_.assign(blockSize, 0.0);
        blockEnergy_.assign(static_cast<std::size_t>(maximumBlocks64), 0.0);
    } catch (...) {
        energyRing_.clear();
        blockEnergy_.clear();
        state_ = StreamingLoudnessState::unprepared;
        return false;
    }

    sampleRate_ = sampleRate;
    blockSize_ = blockSize;
    hopSize_ = hopSize;
    maximumSamples_ = maximumSamples;
    leftFilter_.shelf = makeHighShelf(sampleRate);
    leftFilter_.highPass = makeHighPass(sampleRate);
    rightFilter_ = leftFilter_;
    state_ = StreamingLoudnessState::idle;
    reset();
    return true;
}

void StreamingLoudness1770::reset() noexcept
{
    if (energyRing_.empty() || blockEnergy_.empty() || !(sampleRate_ > 0.0)) {
        state_ = StreamingLoudnessState::unprepared;
        return;
    }

    std::fill(energyRing_.begin(), energyRing_.end(), 0.0);
    std::fill(blockEnergy_.begin(), blockEnergy_.end(), 0.0);
    ringIndex_ = 0;
    blockCount_ = 0;
    runningEnergy_ = 0.0;
    samplesProcessed_ = 0;
    leftFilter_.clear();
    rightFilter_.clear();
    state_ = StreamingLoudnessState::idle;
}

bool StreamingLoudness1770::begin() noexcept
{
    if (state_ == StreamingLoudnessState::unprepared)
        return false;
    reset();
    if (state_ == StreamingLoudnessState::unprepared)
        return false;
    state_ = StreamingLoudnessState::measuring;
    return true;
}

void StreamingLoudness1770::processStereo(
    const std::span<const float> left,
    const std::span<const float> right) noexcept
{
    if (state_ != StreamingLoudnessState::measuring)
        return;

    const auto count = std::min(left.size(), right.size());
    for (std::size_t i = 0; i < count; ++i) {
        if (samplesProcessed_ >= maximumSamples_) {
            state_ = StreamingLoudnessState::capacityExceeded;
            return;
        }

        const auto filteredLeft = leftFilter_.process(static_cast<double>(left[i]));
        const auto filteredRight = rightFilter_.process(static_cast<double>(right[i]));
        const auto energy = filteredLeft * filteredLeft + filteredRight * filteredRight;

        if (samplesProcessed_ < blockSize_) {
            energyRing_[ringIndex_] = energy;
            runningEnergy_ += energy;
        } else {
            runningEnergy_ -= energyRing_[ringIndex_];
            energyRing_[ringIndex_] = energy;
            runningEnergy_ += energy;
        }
        ringIndex_ = (ringIndex_ + 1) % blockSize_;
        ++samplesProcessed_;

        if (samplesProcessed_ >= blockSize_
            && ((samplesProcessed_ - blockSize_) % hopSize_) == 0) {
            if (blockCount_ >= blockEnergy_.size()) {
                state_ = StreamingLoudnessState::capacityExceeded;
                return;
            }
            blockEnergy_[blockCount_++] = runningEnergy_ / static_cast<double>(blockSize_);
        }
    }
}

LoudnessResult StreamingLoudness1770::calculateResult() const noexcept
{
    LoudnessResult result;
    result.totalBlocks = blockCount_;
    if (blockCount_ == 0)
        return result;

    double ungatedEnergy = 0.0;
    for (std::size_t i = 0; i < blockCount_; ++i)
        ungatedEnergy += blockEnergy_[i];
    ungatedEnergy /= static_cast<double>(blockCount_);
    result.ungatedLufs = energyToLufs(ungatedEnergy);

    const auto absoluteEnergy = lufsToEnergy(absoluteGateLufs);
    double absoluteSum = 0.0;
    std::size_t absoluteCount = 0;
    for (std::size_t i = 0; i < blockCount_; ++i) {
        if (blockEnergy_[i] >= absoluteEnergy) {
            absoluteSum += blockEnergy_[i];
            ++absoluteCount;
        }
    }
    if (absoluteCount == 0)
        return result;

    const auto absoluteMean = absoluteSum / static_cast<double>(absoluteCount);
    result.relativeGateLufs = energyToLufs(absoluteMean) - 10.0;
    const auto relativeEnergy = lufsToEnergy(result.relativeGateLufs);

    double finalEnergy = 0.0;
    std::size_t finalCount = 0;
    for (std::size_t i = 0; i < blockCount_; ++i) {
        const auto energy = blockEnergy_[i];
        if (energy >= absoluteEnergy && energy >= relativeEnergy) {
            finalEnergy += energy;
            ++finalCount;
        }
    }
    if (finalCount == 0)
        return result;

    result.gatedBlocks = finalCount;
    result.integratedLufs = energyToLufs(finalEnergy / static_cast<double>(finalCount));
    result.valid = std::isfinite(result.integratedLufs);
    return result;
}

StreamingLoudnessResult StreamingLoudness1770::finish() noexcept
{
    if (state_ == StreamingLoudnessState::measuring)
        state_ = StreamingLoudnessState::complete;

    StreamingLoudnessResult result;
    result.state = state_;
    result.samplesProcessed = samplesProcessed_;
    result.maximumSamples = maximumSamples_;
    result.loudness = calculateResult();
    return result;
}

} // namespace ehvota::analysis
