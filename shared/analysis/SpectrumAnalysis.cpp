#include "SpectrumAnalysis.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>

namespace ehvota::analysis {
namespace {

bool isPowerOfTwo(const std::size_t value) noexcept
{
    return value >= 2 && (value & (value - 1)) == 0;
}

void fft(std::vector<std::complex<double>>& data)
{
    const auto n = data.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(data[i], data[j]);
    }

    for (std::size_t length = 2; length <= n; length <<= 1) {
        const auto angle = -2.0 * std::numbers::pi / static_cast<double>(length);
        const std::complex<double> root(std::cos(angle), std::sin(angle));
        for (std::size_t start = 0; start < n; start += length) {
            std::complex<double> w(1.0, 0.0);
            const auto half = length >> 1;
            for (std::size_t offset = 0; offset < half; ++offset) {
                const auto even = data[start + offset];
                const auto odd = data[start + offset + half] * w;
                data[start + offset] = even + odd;
                data[start + offset + half] = even - odd;
                w *= root;
            }
        }
    }
}

double powerToDb(const double power) noexcept
{
    return power > 1.0e-16 ? 10.0 * std::log10(power) : -160.0;
}

} // namespace

SpectrumSummary analyzeMonoSpectrum(
    const std::span<const float> samples,
    const double sampleRate,
    const std::size_t fftSize,
    const std::size_t hopSize)
{
    SpectrumSummary summary;
    summary.fftSize = fftSize;
    summary.sampleRate = sampleRate;

    if (!isPowerOfTwo(fftSize) || hopSize == 0 || sampleRate <= 0.0 || samples.size() < fftSize)
        return summary;

    const auto binCount = fftSize / 2 + 1;
    std::vector<double> accumulatedPower(binCount, 0.0);
    std::vector<std::complex<double>> frame(fftSize);
    std::vector<double> window(fftSize);

    double windowEnergy = 0.0;
    for (std::size_t i = 0; i < fftSize; ++i) {
        const auto value = 0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(fftSize - 1));
        window[i] = value;
        windowEnergy += value * value;
    }

    for (std::size_t start = 0; start + fftSize <= samples.size(); start += hopSize) {
        for (std::size_t i = 0; i < fftSize; ++i)
            frame[i] = std::complex<double>(static_cast<double>(samples[start + i]) * window[i], 0.0);

        fft(frame);
        for (std::size_t bin = 0; bin < binCount; ++bin) {
            auto power = std::norm(frame[bin]) / windowEnergy;
            if (bin != 0 && bin + 1 != binCount)
                power *= 2.0;
            accumulatedPower[bin] += power;
        }
        ++summary.frameCount;
    }

    if (summary.frameCount == 0)
        return summary;

    summary.bins.reserve(binCount);
    for (std::size_t bin = 0; bin < binCount; ++bin) {
        const auto average = accumulatedPower[bin] / static_cast<double>(summary.frameCount);
        summary.bins.push_back({
            static_cast<double>(bin) * sampleRate / static_cast<double>(fftSize),
            powerToDb(average)
        });
    }
    return summary;
}

BandEnergy summarizeBand(const SpectrumSummary& spectrum, const double lowHz, const double highHz)
{
    BandEnergy band { lowHz, highHz, -160.0 };
    if (spectrum.bins.empty() || highHz <= lowHz)
        return band;

    double totalPower = 0.0;
    std::size_t count = 0;
    for (const auto& bin : spectrum.bins) {
        if (bin.frequencyHz < lowHz || bin.frequencyHz > highHz)
            continue;
        totalPower += std::pow(10.0, bin.powerDb / 10.0);
        ++count;
    }

    if (count > 0)
        band.averagePowerDb = powerToDb(totalPower / static_cast<double>(count));
    return band;
}

double compareBandDb(
    const SpectrumSummary& mix,
    const SpectrumSummary& reference,
    const double lowHz,
    const double highHz)
{
    const auto mixBand = summarizeBand(mix, lowHz, highHz);
    const auto referenceBand = summarizeBand(reference, lowHz, highHz);
    return mixBand.averagePowerDb - referenceBand.averagePowerDb;
}

} // namespace ehvota::analysis
