#include "shared/analysis/CapturedSectionAnalysis.h"
#include "shared/analysis/ComparisonAnalysis.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {
constexpr double pi = 3.14159265358979323846;

void require(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

std::vector<float> sine(double sampleRate, double seconds, double amplitude)
{
    const auto count = static_cast<std::size_t>(sampleRate * seconds);
    std::vector<float> values(count);
    for (std::size_t i = 0; i < count; ++i)
        values[i] = static_cast<float>(amplitude * std::sin(2.0 * pi * 1000.0 * static_cast<double>(i) / sampleRate));
    return values;
}
} // namespace

int main()
{
    constexpr double sampleRate = 48000.0;
    const auto referenceAudio = sine(sampleRate, 2.0, 0.10);
    const auto mixAudio = sine(sampleRate, 2.0, 0.20);

    const auto reference = ehvota::analysis::analyzeCapturedSection(
        "chorus-1", referenceAudio, referenceAudio, sampleRate);
    const auto mix = ehvota::analysis::analyzeCapturedSection(
        "chorus-1", mixAudio, mixAudio, sampleRate);
    require(reference.valid && mix.valid, "two-second test sections should analyze successfully");

    const auto comparison = ehvota::analysis::compareCapturedSections(mix, reference);
    require(comparison.valid, "valid section analyses should compare successfully");
    require(std::abs(comparison.dynamics.loudnessLu - 6.0206) < 0.10,
        "doubling amplitude should produce approximately +6.02 LU loudness difference");

    const auto& midBand = comparison.bands[2];
    require(midBand.band.id == ehvota::analysis::ComparisonBand::mid,
        "third default comparison band should be MID");
    require(midBand.differenceDb > 5.8 && midBand.differenceDb < 6.3,
        "1 kHz spectral level difference should be represented in MID band");
    require(std::abs(comparison.space.correlation) < 1.0e-6,
        "identical stereo geometry at a different gain should not create a correlation difference");

    std::cout << "Comparison analysis tests passed\n";
    return 0;
}
