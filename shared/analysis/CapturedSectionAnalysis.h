#pragma once

#include "BasicMetrics.h"
#include "Loudness1770.h"
#include "SpectrumAnalysis.h"

#include <span>
#include <string>

namespace ehvota::analysis {

struct CapturedSectionAnalysis {
    bool valid {};
    std::string sectionId;
    double durationSeconds {};
    StereoMetrics stereo;
    LoudnessResult loudness;
    SpectrumSummary spectrum;
};

[[nodiscard]] CapturedSectionAnalysis analyzeCapturedSection(
    std::string sectionId,
    std::span<const float> left,
    std::span<const float> right,
    double sampleRate,
    std::size_t fftSize = 4096,
    std::size_t hopSize = 1024);

} // namespace ehvota::analysis
