#include "shared/viewmodels/LevelMatchState.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

using ehvota::preview::EvidenceQuality;
using ehvota::preview::LevelMatchMeasurement;
using ehvota::preview::LevelMatchMode;
using ehvota::preview::LevelMatchState;
using ehvota::preview::ResolvedLevelMatch;
using ehvota::preview::resolveLevelMatch;

void require(const bool condition, const std::string_view message)
{
    if (condition)
        return;

    std::cerr << "LevelMatchStateTests: " << message << '\n';
    std::exit(EXIT_FAILURE);
}

bool near(const double actual, const double expected)
{
    return std::abs(actual - expected) < 0.0001;
}

LevelMatchMeasurement stable(
    const double mixLufs,
    const double referenceLufs,
    const double trimDb,
    std::string explanation = {})
{
    return {
        mixLufs,
        referenceLufs,
        trimDb,
        EvidenceQuality::capturedStable,
        std::move(explanation)
    };
}

void testStableSectionWins()
{
    LevelMatchState state;
    state.section = stable(-14.8, -10.9, -3.9);
    state.wholeTrack = stable(-13.7, -10.5, -3.2);

    const auto resolved = resolveLevelMatch(state);
    require(resolved.enabled, "stable section match should be enabled");
    require(resolved.effectiveMode == LevelMatchMode::autoSection, "stable section evidence should remain Auto / Section");
    require(resolved.measurement.referenceMonitorTrimDb.has_value(), "stable section match should expose monitor trim");
    require(near(*resolved.measurement.referenceMonitorTrimDb, -3.9), "stable section trim should be preserved");
    require(resolved.measurement.evidence == EvidenceQuality::capturedStable, "stable section evidence should remain captured/stable");
}

void testUnstableSectionFallsBackExplicitly()
{
    LevelMatchState state;
    state.section = {
        -15.0,
        -11.0,
        -4.0,
        EvidenceQuality::liveTemporary,
        "Live values are provisional."
    };
    state.wholeTrack = stable(-13.7, -10.5, -3.2);

    const auto resolved = resolveLevelMatch(state);
    require(resolved.enabled, "whole-track fallback should remain usable");
    require(resolved.effectiveMode == LevelMatchMode::wholeTrack, "fallback must be labelled Whole Track");
    require(resolved.measurement.evidence == EvidenceQuality::fallbackWholeTrack, "fallback evidence must be explicit");
    require(near(*resolved.measurement.referenceMonitorTrimDb, -3.2), "fallback trim should use whole-track evidence");
}

void testNoEvidenceDoesNotInventMatch()
{
    LevelMatchState state;
    const auto resolved = resolveLevelMatch(state);

    require(!resolved.enabled, "missing evidence must disable automatic matching rather than invent a value");
    require(!resolved.measurement.referenceMonitorTrimDb.has_value(), "missing evidence must not expose a fabricated trim");
    require(resolved.measurement.evidence == EvidenceQuality::unavailable, "missing evidence must be labelled unavailable");
}

void testOffPreservesOriginalRelativeLevel()
{
    LevelMatchState state;
    state.mode = LevelMatchMode::off;
    state.enabled = false;
    state.section = stable(-14.8, -10.9, -3.9);

    const auto resolved = resolveLevelMatch(state);
    require(!resolved.enabled, "Off mode must disable monitoring compensation");
    require(resolved.effectiveMode == LevelMatchMode::off, "Off mode must remain Off");
    require(!resolved.measurement.referenceMonitorTrimDb.has_value(), "Off mode must not apply a monitor trim");
}

void testBlindComparisonForcesMatchingOn()
{
    LevelMatchState state;
    state.enabled = false;
    state.blindComparison = true;
    state.section = stable(-14.8, -10.9, -3.9);

    const auto resolved = resolveLevelMatch(state);
    require(resolved.enabled, "blind comparison must default to level matching");
    require(resolved.forcedByBlindComparison, "blind comparison should report that it forced matching on");
    require(resolved.effectiveMode == LevelMatchMode::autoSection, "blind comparison should use the stable selected section when available");
}

} // namespace

int main()
{
    testStableSectionWins();
    testUnstableSectionFallsBackExplicitly();
    testNoEvidenceDoesNotInventMatch();
    testOffPreservesOriginalRelativeLevel();
    testBlindComparisonForcesMatchingOn();
    return EXIT_SUCCESS;
}
