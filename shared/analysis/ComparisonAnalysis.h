#pragma once

#include "CapturedSectionAnalysis.h"

#include <array>
#include <string_view>

namespace ehvota::analysis {

enum class ComparisonBand {
    low,
    lowMid,
    mid,
    highMid,
    high,
    air
};

struct BandDefinition {
    ComparisonBand id;
    std::string_view name;
    double lowHz {};
    double highHz {};
};

struct BandDifference {
    BandDefinition band;
    double mixPowerDb {-160.0};
    double referencePowerDb {-160.0};
    double differenceDb {};
};

struct DynamicsDifference {
    double loudnessLu {};
    double rmsDb {};
    double crestDb {};
};

struct SpaceDifference {
    double correlation {};
    double sideToMidDb {};
};

struct SectionComparison {
    bool valid {};
    std::array<BandDifference, 6> bands;
    DynamicsDifference dynamics;
    SpaceDifference space;
};

[[nodiscard]] const std::array<BandDefinition, 6>& defaultComparisonBands() noexcept;
[[nodiscard]] SectionComparison compareCapturedSections(
    const CapturedSectionAnalysis& mix,
    const CapturedSectionAnalysis& reference);

} // namespace ehvota::analysis
