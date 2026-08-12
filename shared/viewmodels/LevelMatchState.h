#pragma once

#include <optional>
#include <string>

namespace ehvota::preview {

enum class LevelMatchMode {
    autoSection,
    wholeTrack,
    off
};

enum class EvidenceQuality {
    unavailable,
    liveTemporary,
    capturedStable,
    fallbackWholeTrack,
    stale,
    degraded
};

struct LevelMatchMeasurement {
    std::optional<double> mixLufs;
    std::optional<double> referenceLufs;
    std::optional<double> referenceMonitorTrimDb;
    EvidenceQuality evidence { EvidenceQuality::unavailable };
    std::string explanation;
};

struct LevelMatchState {
    LevelMatchMode mode { LevelMatchMode::autoSection };
    bool enabled { true };
    bool blindComparison { false };
    LevelMatchMeasurement section;
    LevelMatchMeasurement wholeTrack;
};

struct ResolvedLevelMatch {
    LevelMatchMode requestedMode { LevelMatchMode::autoSection };
    LevelMatchMode effectiveMode { LevelMatchMode::off };
    bool enabled { false };
    bool forcedByBlindComparison { false };
    LevelMatchMeasurement measurement;
};

[[nodiscard]] ResolvedLevelMatch resolveLevelMatch(const LevelMatchState& state);
[[nodiscard]] std::string levelMatchModeName(LevelMatchMode mode);
[[nodiscard]] std::string evidenceQualityName(EvidenceQuality quality);

} // namespace ehvota::preview
