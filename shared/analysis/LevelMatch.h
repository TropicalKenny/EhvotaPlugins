#pragma once

#include "Loudness1770.h"

namespace ehvota::analysis {

struct LevelMatchDecision {
    bool valid {};
    double mixLufs {-100.0};
    double referenceLufs {-100.0};
    double referenceMonitorTrimDb {};
};

[[nodiscard]] LevelMatchDecision calculateReferenceMonitorTrim(
    const LoudnessResult& mix,
    const LoudnessResult& reference,
    double maximumAbsoluteTrimDb = 24.0);

} // namespace ehvota::analysis
