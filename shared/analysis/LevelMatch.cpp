#include "LevelMatch.h"

#include <algorithm>
#include <cmath>

namespace ehvota::analysis {

LevelMatchDecision calculateReferenceMonitorTrim(
    const LoudnessResult& mix,
    const LoudnessResult& reference,
    double maximumAbsoluteTrimDb)
{
    LevelMatchDecision result;
    if (!mix.valid || !reference.valid || maximumAbsoluteTrimDb <= 0.0)
        return result;

    result.mixLufs = mix.integratedLufs;
    result.referenceLufs = reference.integratedLufs;
    const double requested = result.mixLufs - result.referenceLufs;
    result.referenceMonitorTrimDb = std::clamp(
        requested,
        -std::abs(maximumAbsoluteTrimDb),
        std::abs(maximumAbsoluteTrimDb));
    result.valid = std::isfinite(result.referenceMonitorTrimDb);
    return result;
}

} // namespace ehvota::analysis
