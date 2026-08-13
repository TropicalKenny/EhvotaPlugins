#include "shared/state/CapturedComparison.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

void require(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

ehvota::state::CaptureMeasurement measurement(
    const char* id,
    const char* section,
    double lufs,
    ehvota::state::CaptureEvidence evidence)
{
    ehvota::state::CaptureMeasurement value;
    value.id = id;
    value.sectionId = section;
    value.durationSeconds = 36.0;
    value.evidence = evidence;
    value.loudness.valid = true;
    value.loudness.integratedLufs = lufs;
    return value;
}

} // namespace

int main()
{
    ehvota::state::CapturedComparison comparison;
    comparison.mix = measurement("mix-chorus", "chorus-1", -14.8, ehvota::state::CaptureEvidence::capturedStable);
    comparison.reference = measurement("ref-chorus", "chorus-1", -10.9, ehvota::state::CaptureEvidence::capturedStable);

    require(comparison.hasStablePair(), "matching stable section captures should form a valid pair");
    const auto decision = comparison.levelMatch();
    require(decision.valid, "stable pair should produce level match");
    require(std::abs(decision.referenceMonitorTrimDb + 3.9) < 0.001,
        "reference trim must align the reference loudness to the mix");

    comparison.reference->sectionId = "chorus-2";
    require(!comparison.hasStablePair(), "different musical sections must not silently level match");
    require(!comparison.levelMatch().valid, "mismatched section IDs must not produce trim");

    comparison.reference->sectionId = "chorus-1";
    comparison.reference->evidence = ehvota::state::CaptureEvidence::stale;
    require(!comparison.hasStablePair(), "stale evidence must not be treated as captured stable evidence");

    std::cout << "Captured comparison tests passed\n";
    return 0;
}
