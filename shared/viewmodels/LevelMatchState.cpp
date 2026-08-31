#include "LevelMatchState.h"

namespace ehvota::preview {
namespace {

bool hasUsableMeasurement(const LevelMatchMeasurement& measurement)
{
    if (!measurement.mixLufs.has_value()
        || !measurement.referenceLufs.has_value()
        || !measurement.referenceMonitorTrimDb.has_value()) {
        return false;
    }

    return measurement.evidence == EvidenceQuality::capturedStable
        || measurement.evidence == EvidenceQuality::fallbackWholeTrack;
}

LevelMatchMeasurement unavailableMeasurement(std::string explanation)
{
    LevelMatchMeasurement result;
    result.evidence = EvidenceQuality::unavailable;
    result.explanation = std::move(explanation);
    return result;
}

} // namespace

ResolvedLevelMatch resolveLevelMatch(const LevelMatchState& state)
{
    ResolvedLevelMatch result;
    result.requestedMode = state.mode;
    result.forcedByBlindComparison = state.blindComparison && !state.enabled;

    const auto enabled = state.enabled || state.blindComparison;
    if (!enabled || state.mode == LevelMatchMode::off) {
        result.effectiveMode = LevelMatchMode::off;
        result.enabled = false;
        result.measurement = unavailableMeasurement("Level matching is off; original relative loudness is preserved.");
        return result;
    }

    result.enabled = true;

    if (state.mode == LevelMatchMode::wholeTrack) {
        if (hasUsableMeasurement(state.wholeTrack)) {
            result.effectiveMode = LevelMatchMode::wholeTrack;
            result.measurement = state.wholeTrack;
            return result;
        }

        result.effectiveMode = LevelMatchMode::wholeTrack;
        result.enabled = false;
        result.measurement = unavailableMeasurement(
            "Whole-track level match is unavailable because stable whole-track evidence is missing.");
        return result;
    }

    if (hasUsableMeasurement(state.section)
        && state.section.evidence == EvidenceQuality::capturedStable) {
        result.effectiveMode = LevelMatchMode::autoSection;
        result.measurement = state.section;
        return result;
    }

    if (hasUsableMeasurement(state.wholeTrack)) {
        result.effectiveMode = LevelMatchMode::wholeTrack;
        result.measurement = state.wholeTrack;
        result.measurement.evidence = EvidenceQuality::fallbackWholeTrack;
        if (result.measurement.explanation.empty()) {
            result.measurement.explanation =
                "Section evidence is not stable; using the validated whole-track match as a labelled fallback.";
        }
        return result;
    }

    result.effectiveMode = LevelMatchMode::autoSection;
    result.enabled = false;
    result.measurement = unavailableMeasurement(
        "Section level match is unavailable and no valid whole-track fallback exists.");
    return result;
}

std::string levelMatchModeName(const LevelMatchMode mode)
{
    switch (mode) {
    case LevelMatchMode::autoSection:
        return "Auto / Section";
    case LevelMatchMode::wholeTrack:
        return "Whole Track";
    case LevelMatchMode::off:
        return "Off";
    }

    return "Unknown";
}

std::string evidenceQualityName(const EvidenceQuality quality)
{
    switch (quality) {
    case EvidenceQuality::unavailable:
        return "Unavailable";
    case EvidenceQuality::liveTemporary:
        return "Live / temporary";
    case EvidenceQuality::capturedStable:
        return "Captured / stable";
    case EvidenceQuality::fallbackWholeTrack:
        return "Whole-track fallback";
    case EvidenceQuality::stale:
        return "Stale";
    case EvidenceQuality::degraded:
        return "Degraded";
    }

    return "Unknown";
}

} // namespace ehvota::preview
