#pragma once

#include "CapturedSectionAnalysis.h"
#include "ComparisonAnalysis.h"

#include <array>

namespace ehvota::analysis {

// Transport-neutral summary used whenever raw captured audio must stay local to a plugin instance.
struct CompactSectionAnalysis {
    bool valid {};
    double integratedLufs { -100.0 };
    double peakDb { -120.0 };
    double rmsDb { -120.0 };
    double crestDb {};
    double correlation { 1.0 };
    double sideToMidDb { -120.0 };
    std::array<double, 6> bandPowerDb { -160.0, -160.0, -160.0, -160.0, -160.0, -160.0 };
};

[[nodiscard]] CompactSectionAnalysis compactCapturedSection(
    const CapturedSectionAnalysis& analysis);

[[nodiscard]] SectionComparison compareCompactSections(
    const CompactSectionAnalysis& mix,
    const CompactSectionAnalysis& reference);

} // namespace ehvota::analysis
