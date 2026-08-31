#include "Loudness1770.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <numeric>

namespace ehvota::analysis {
namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double loudnessOffset = -0.691;
constexpr double absoluteGate = -70.0;

struct Biquad {
    double b0 {1.0};
    double b1 {};
    double b2 {};
    double a1 {};
    double a2 {};
    double z1 {};
    double z2 {};

    double process(double x)
    {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
};

Biquad makeHighShelf(double sampleRate, double frequency, double gainDb, double q)
{
    const double a = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * pi * frequency / sampleRate;
    const double alpha = std::sin(w0) / (2.0 * q);
    const double cosw = std::cos(w0);
    const double sqrtA = std::sqrt(a);

    const double b0 = a * ((a + 1.0) + (a - 1.0) * cosw + 2.0 * sqrtA * alpha);
    const double b1 = -2.0 * a * ((a - 1.0) + (a + 1.0) * cosw);
    const double b2 = a * ((a + 1.0) + (a - 1.0) * cosw - 2.0 * sqrtA * alpha);
    const double a0 = (a + 1.0) - (a - 1.0) * cosw + 2.0 * sqrtA * alpha;
    const double a1 = 2.0 * ((a - 1.0) - (a + 1.0) * cosw);
    const double a2 = (a + 1.0) - (a - 1.0) * cosw - 2.0 * sqrtA * alpha;

    return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
}

Biquad makeHighPass(double sampleRate, double frequency, double q)
{
    const double w0 = 2.0 * pi * frequency / sampleRate;
    const double alpha = std::sin(w0) / (2.0 * q);
    const double cosw = std::cos(w0);
    const double b0 = (1.0 + cosw) / 2.0;
    const double b1 = -(1.0 + cosw);
    const double b2 = (1.0 + cosw) / 2.0;
    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cosw;
    const double a2 = 1.0 - alpha;
    return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
}

double energyToLufs(double meanSquare)
{
    if (meanSquare <= 0.0)
        return -100.0;
    return loudnessOffset + 10.0 * std::log10(meanSquare);
}

double lufsToEnergy(double lufs)
{
    return std::pow(10.0, (lufs - loudnessOffset) / 10.0);
}

struct ChannelKWeighting {
    Biquad shelf;
    Biquad highPass;

    explicit ChannelKWeighting(double sampleRate)
        : shelf(makeHighShelf(sampleRate, 1681.974450955533, 3.999843853973347, 0.7071752369554196)),
          highPass(makeHighPass(sampleRate, 38.13547087602444, 0.5003270373238773))
    {
    }

    double process(double sample)
    {
        return highPass.process(shelf.process(sample));
    }
};

} // namespace

Loudness1770::Loudness1770(double sampleRate)
    : sampleRate_(sampleRate)
{
}

LoudnessResult Loudness1770::measureStereo(
    std::span<const float> left,
    std::span<const float> right) const
{
    LoudnessResult result;
    if (sampleRate_ < 8000.0 || left.empty() || right.empty())
        return result;

    const auto sampleCount = std::min(left.size(), right.size());
    const auto blockSize = static_cast<std::size_t>(std::llround(sampleRate_ * 0.400));
    const auto hopSize = static_cast<std::size_t>(std::llround(sampleRate_ * 0.100));
    if (blockSize == 0 || hopSize == 0 || sampleCount < blockSize)
        return result;

    ChannelKWeighting filterL(sampleRate_);
    ChannelKWeighting filterR(sampleRate_);
    std::vector<double> weightedEnergy(sampleCount);
    for (std::size_t i = 0; i < sampleCount; ++i) {
        const double l = filterL.process(static_cast<double>(left[i]));
        const double r = filterR.process(static_cast<double>(right[i]));
        weightedEnergy[i] = l * l + r * r;
    }

    std::vector<double> prefix(sampleCount + 1, 0.0);
    for (std::size_t i = 0; i < sampleCount; ++i)
        prefix[i + 1] = prefix[i] + weightedEnergy[i];

    std::vector<double> blockEnergy;
    for (std::size_t start = 0; start + blockSize <= sampleCount; start += hopSize) {
        const double sum = prefix[start + blockSize] - prefix[start];
        blockEnergy.push_back(sum / static_cast<double>(blockSize));
    }
    result.totalBlocks = blockEnergy.size();
    if (blockEnergy.empty())
        return result;

    const double ungatedMean = std::accumulate(blockEnergy.begin(), blockEnergy.end(), 0.0) /
        static_cast<double>(blockEnergy.size());
    result.ungatedLufs = energyToLufs(ungatedMean);

    std::vector<double> absoluteGated;
    absoluteGated.reserve(blockEnergy.size());
    const double absoluteEnergy = lufsToEnergy(absoluteGate);
    std::copy_if(blockEnergy.begin(), blockEnergy.end(), std::back_inserter(absoluteGated),
        [absoluteEnergy](double energy) { return energy >= absoluteEnergy; });
    if (absoluteGated.empty())
        return result;

    const double absoluteMean = std::accumulate(absoluteGated.begin(), absoluteGated.end(), 0.0) /
        static_cast<double>(absoluteGated.size());
    const double absoluteMeanLufs = energyToLufs(absoluteMean);
    result.relativeGateLufs = absoluteMeanLufs - 10.0;
    const double relativeEnergy = lufsToEnergy(result.relativeGateLufs);

    double finalEnergy = 0.0;
    std::size_t finalCount = 0;
    for (double energy : absoluteGated) {
        if (energy >= relativeEnergy) {
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

} // namespace ehvota::analysis
