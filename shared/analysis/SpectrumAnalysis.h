#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace ehvota::analysis {

struct SpectrumBin {
    double frequencyHz {};
    double powerDb { -160.0 };
};

struct SpectrumSummary {
    std::size_t fftSize {};
    std::size_t frameCount {};
    double sampleRate {};
    std::vector<SpectrumBin> bins;
};

struct BandEnergy {
    double lowHz {};
    double highHz {};
    double averagePowerDb { -160.0 };
};

SpectrumSummary analyzeMonoSpectrum(
    std::span<const float> samples,
    double sampleRate,
    std::size_t fftSize = 4096,
    std::size_t hopSize = 1024);

BandEnergy summarizeBand(const SpectrumSummary& spectrum, double lowHz, double highHz);

double compareBandDb(
    const SpectrumSummary& mix,
    const SpectrumSummary& reference,
    double lowHz,
    double highHz);

} // namespace ehvota::analysis
