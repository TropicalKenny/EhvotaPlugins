#pragma once

#include "shared/analysis/LevelMatch.h"

#include <optional>
#include <string>

namespace ehvota::state {

enum class CaptureEvidence {
    unavailable,
    liveTemporary,
    capturedStable,
    stale
};

struct CaptureMeasurement {
    std::string id;
    std::string sectionId;
    double durationSeconds {};
    ehvota::analysis::LoudnessResult loudness;
    CaptureEvidence evidence { CaptureEvidence::unavailable };
};

struct CapturedComparison {
    std::optional<CaptureMeasurement> mix;
    std::optional<CaptureMeasurement> reference;

    [[nodiscard]] bool hasStablePair() const noexcept;
    [[nodiscard]] ehvota::analysis::LevelMatchDecision levelMatch() const noexcept;
};

} // namespace ehvota::state
