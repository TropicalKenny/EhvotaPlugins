#include "shared/analysis/BasicMetrics.h"
#include "shared/analysis/SpectrumAnalysis.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numbers>
#include <string_view>
#include <vector>

namespace {
void require(bool condition, std::string_view message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
}

int main()
{
    using namespace ehvota::analysis;

    constexpr double sampleRate = 48000.0;
    std::vector<float> left(48000);
    std::vector<float> right(48000);
    for (std::size_t i = 0; i < left.size(); ++i) {
        const auto phase = 2.0 * std::numbers::pi * 1000.0 * static_cast<double>(i) / sampleRate;
        left[i] = static_cast<float>(0.5 * std::sin(phase));
        right[i] = left[i];
    }

    const auto stereo = calculateStereoMetrics(left, right);
    require(std::abs(stereo.peakDbfs + 6.0206) < 0.05, "0.5 sine peak should be about -6.02 dBFS");
    require(stereo.correlation > 0.999, "identical channels should correlate near +1");
    require(stereo.sideRmsDbfs <= -119.0, "identical channels should have negligible side energy");

    const auto spectrum = analyzeMonoSpectrum(left, sampleRate, 4096, 1024);
    require(spectrum.frameCount > 0, "spectrum should contain frames");
    const auto toneBand = summarizeBand(spectrum, 900.0, 1100.0);
    const auto distantBand = summarizeBand(spectrum, 4000.0, 5000.0);
    require(toneBand.averagePowerDb > distantBand.averagePowerDb + 30.0, "1 kHz tone should dominate distant band");

    auto louder = left;
    for (auto& sample : louder)
        sample *= 2.0f;
    const auto louderSpectrum = analyzeMonoSpectrum(louder, sampleRate, 4096, 1024);
    const auto difference = compareBandDb(louderSpectrum, spectrum, 900.0, 1100.0);
    require(std::abs(difference - 6.0206) < 0.2, "doubling amplitude should be about +6.02 dB in band power");

    std::cout << "Analysis tests passed.\n";
    return EXIT_SUCCESS;
}
