#include "CompactAnalysis.h"

#include <cmath>

namespace ehvota::analysis {

CompactSectionAnalysis compactCapturedSection(const CapturedSectionAnalysis& analysis)
{
    CompactSectionAnalysis result;
    if (!analysis.valid)
        return result;

    result.integratedLufs = analysis.loudness.integratedLufs;
    result.peakDb = analysis.stereo.peakDbfs;
    result.rmsDb = analysis.stereo.rmsDbfs;
    result.crestDb = analysis.stereo.crestFactorDb;
    result.correlation = analysis.stereo.correlation;
    result.sideToMidDb = analysis.stereo.sideToMidDb;

    const auto& definitions = defaultComparisonBands();
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        result.bandPowerDb[i] = summarizeBand(
            analysis.spectrum, definitions[i].lowHz, definitions[i].highHz).averagePowerDb;
        if (!std::isfinite(result.bandPowerDb[i]))
            return CompactSectionAnalysis{};
    }

    result.valid = std::isfinite(result.integratedLufs)
        && std::isfinite(result.peakDb)
        && std::isfinite(result.rmsDb)
        && std::isfinite(result.crestDb)
        && std::isfinite(result.correlation)
        && std::isfinite(result.sideToMidDb);
    return result;
}

SectionComparison compareCompactSections(
    const CompactSectionAnalysis& mix,
    const CompactSectionAnalysis& reference)
{
    SectionComparison result;
    const auto& definitions = defaultComparisonBands();
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        result.bands[i] = {
            definitions[i],
            mix.bandPowerDb[i],
            reference.bandPowerDb[i],
            mix.bandPowerDb[i] - reference.bandPowerDb[i]
        };
    }

    if (!mix.valid || !reference.valid)
        return result;

    result.dynamics.loudnessLu = mix.integratedLufs - reference.integratedLufs;
    result.dynamics.rmsDb = mix.rmsDb - reference.rmsDb;
    result.dynamics.crestDb = mix.crestDb - reference.crestDb;
    result.space.correlation = mix.correlation - reference.correlation;
    result.space.sideToMidDb = mix.sideToMidDb - reference.sideToMidDb;
    result.valid = true;
    return result;
}

} // namespace ehvota::analysis
