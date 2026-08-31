#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace ehvota::analysis {

struct StereoMetrics {
    double peakDbfs { -120.0 };
    double rmsDbfs { -120.0 };
    double crestFactorDb {};
    double correlation { 1.0 };
    double midRmsDbfs { -120.0 };
    double sideRmsDbfs { -120.0 };
    double sideToMidDb { -120.0 };
};

StereoMetrics calculateStereoMetrics(
    std::span<const float> left,
    std::span<const float> right);

StereoMetrics calculateMonoMetrics(std::span<const float> mono);

double gainToDb(double gain, double floorDb = -120.0) noexcept;

} // namespace ehvota::analysis
