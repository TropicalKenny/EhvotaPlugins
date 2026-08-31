#include "ComparisonAnalysis.h"

namespace ehvota::analysis {

const std::array<BandDefinition, 6>& defaultComparisonBands() noexcept
{
    static constexpr std::array<BandDefinition, 6> bands {{
        { ComparisonBand::low, "LOW", 20.0, 120.0 },
        { ComparisonBand::lowMid, "LOW-MID", 120.0, 500.0 },
        { ComparisonBand::mid, "MID", 500.0, 2000.0 },
        { ComparisonBand::highMid, "HIGH-MID", 2000.0, 6000.0 },
        { ComparisonBand::high, "HIGH", 6000.0, 12000.0 },
        { ComparisonBand::air, "AIR", 12000.0, 20000.0 }
    }};
    return bands;
}

SectionComparison compareCapturedSections(
    const CapturedSectionAnalysis& mix,
    const CapturedSectionAnalysis& reference)
{
    SectionComparison result;
    const auto& definitions = defaultComparisonBands();
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        const auto& definition = definitions[i];
        const auto mixBand = summarizeBand(mix.spectrum, definition.lowHz, definition.highHz);
        const auto refBand = summarizeBand(reference.spectrum, definition.lowHz, definition.highHz);
        result.bands[i] = {
            definition,
            mixBand.averagePowerDb,
            refBand.averagePowerDb,
            mixBand.averagePowerDb - refBand.averagePowerDb
        };
    }

    if (!mix.valid || !reference.valid)
        return result;

    result.dynamics.loudnessLu = mix.loudness.integratedLufs - reference.loudness.integratedLufs;
    result.dynamics.rmsDb = mix.stereo.rmsDbfs - reference.stereo.rmsDbfs;
    result.dynamics.crestDb = mix.stereo.crestFactorDb - reference.stereo.crestFactorDb;
    result.space.correlation = mix.stereo.correlation - reference.stereo.correlation;
    result.space.sideToMidDb = mix.stereo.sideToMidDb - reference.stereo.sideToMidDb;
    result.valid = true;
    return result;
}

} // namespace ehvota::analysis
