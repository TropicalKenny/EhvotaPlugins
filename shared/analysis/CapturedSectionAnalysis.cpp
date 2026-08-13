#include "CapturedSectionAnalysis.h"

#include <algorithm>
#include <vector>

namespace ehvota::analysis {

CapturedSectionAnalysis analyzeCapturedSection(
    std::string sectionId,
    std::span<const float> left,
    std::span<const float> right,
    const double sampleRate,
    const std::size_t fftSize,
    const std::size_t hopSize)
{
    CapturedSectionAnalysis result;
    result.sectionId = std::move(sectionId);
    if (!(sampleRate > 0.0) || left.empty() || right.empty())
        return result;

    const auto sampleCount = std::min(left.size(), right.size());
    const auto l = left.first(sampleCount);
    const auto r = right.first(sampleCount);
    result.durationSeconds = static_cast<double>(sampleCount) / sampleRate;
    result.stereo = calculateStereoMetrics(l, r);

    Loudness1770 loudness(sampleRate);
    result.loudness = loudness.measureStereo(l, r);

    std::vector<float> mid(sampleCount);
    for (std::size_t i = 0; i < sampleCount; ++i)
        mid[i] = 0.5f * (l[i] + r[i]);
    result.spectrum = analyzeMonoSpectrum(mid, sampleRate, fftSize, hopSize);

    result.valid = result.loudness.valid && result.spectrum.frameCount > 0;
    return result;
}

} // namespace ehvota::analysis
