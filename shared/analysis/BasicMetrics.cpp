#include "BasicMetrics.h"

#include <algorithm>
#include <cmath>

namespace ehvota::analysis {

double gainToDb(const double gain, const double floorDb) noexcept
{
    if (gain <= 0.0)
        return floorDb;
    return std::max(floorDb, 20.0 * std::log10(gain));
}

StereoMetrics calculateStereoMetrics(const std::span<const float> left, const std::span<const float> right)
{
    const auto count = std::min(left.size(), right.size());
    if (count == 0)
        return {};

    double sumSquares = 0.0;
    double midSquares = 0.0;
    double sideSquares = 0.0;
    double ll = 0.0;
    double rr = 0.0;
    double lr = 0.0;
    double peak = 0.0;

    for (std::size_t i = 0; i < count; ++i) {
        const auto l = static_cast<double>(left[i]);
        const auto r = static_cast<double>(right[i]);
        const auto mid = 0.5 * (l + r);
        const auto side = 0.5 * (l - r);

        peak = std::max({ peak, std::abs(l), std::abs(r) });
        sumSquares += l * l + r * r;
        midSquares += mid * mid;
        sideSquares += side * side;
        ll += l * l;
        rr += r * r;
        lr += l * r;
    }

    const auto rms = std::sqrt(sumSquares / static_cast<double>(count * 2));
    const auto midRms = std::sqrt(midSquares / static_cast<double>(count));
    const auto sideRms = std::sqrt(sideSquares / static_cast<double>(count));
    const auto denominator = std::sqrt(ll * rr);

    StereoMetrics metrics;
    metrics.peakDbfs = gainToDb(peak);
    metrics.rmsDbfs = gainToDb(rms);
    metrics.crestFactorDb = metrics.peakDbfs - metrics.rmsDbfs;
    metrics.correlation = denominator > 1.0e-18 ? std::clamp(lr / denominator, -1.0, 1.0) : 1.0;
    metrics.midRmsDbfs = gainToDb(midRms);
    metrics.sideRmsDbfs = gainToDb(sideRms);
    metrics.sideToMidDb = midRms > 1.0e-18 ? gainToDb(sideRms / midRms) : -120.0;
    return metrics;
}

StereoMetrics calculateMonoMetrics(const std::span<const float> mono)
{
    if (mono.empty())
        return {};

    double sumSquares = 0.0;
    double peak = 0.0;
    for (const auto sample : mono) {
        const auto value = static_cast<double>(sample);
        peak = std::max(peak, std::abs(value));
        sumSquares += value * value;
    }

    const auto rms = std::sqrt(sumSquares / static_cast<double>(mono.size()));
    StereoMetrics metrics;
    metrics.peakDbfs = gainToDb(peak);
    metrics.rmsDbfs = gainToDb(rms);
    metrics.crestFactorDb = metrics.peakDbfs - metrics.rmsDbfs;
    metrics.correlation = 1.0;
    metrics.midRmsDbfs = metrics.rmsDbfs;
    metrics.sideRmsDbfs = -120.0;
    metrics.sideToMidDb = -120.0;
    return metrics;
}

} // namespace ehvota::analysis
