#include "shared/analysis/CompactAnalysis.h"

#include <cassert>
#include <cmath>

int main()
{
    using namespace ehvota::analysis;

    CompactSectionAnalysis mix;
    mix.valid = true;
    mix.integratedLufs = -14.0;
    mix.peakDb = -2.0;
    mix.rmsDb = -16.0;
    mix.crestDb = 14.0;
    mix.correlation = 0.7;
    mix.sideToMidDb = -8.0;
    mix.bandPowerDb = { -20.0, -18.0, -22.0, -24.0, -28.0, -34.0 };

    auto reference = mix;
    reference.integratedLufs = -12.0;
    reference.rmsDb = -14.0;
    reference.crestDb = 12.0;
    reference.correlation = 0.8;
    reference.sideToMidDb = -10.0;
    reference.bandPowerDb[1] = -21.0;

    const auto comparison = compareCompactSections(mix, reference);
    assert(comparison.valid);
    assert(std::abs(comparison.bands[1].differenceDb - 3.0) < 1.0e-9);
    assert(std::abs(comparison.dynamics.loudnessLu + 2.0) < 1.0e-9);
    assert(std::abs(comparison.dynamics.crestDb - 2.0) < 1.0e-9);
    assert(std::abs(comparison.space.correlation + 0.1) < 1.0e-9);
    assert(std::abs(comparison.space.sideToMidDb - 2.0) < 1.0e-9);

    reference.valid = false;
    assert(!compareCompactSections(mix, reference).valid);
    return 0;
}
