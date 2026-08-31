#include "shared/audio/AnalogSaturation.h"

#include <cassert>
#include <cmath>
#include <vector>

int main()
{
    using namespace ehvota::audio;

    // Clean state must be numerically transparent apart from an explicitly requested output gain.
    AnalogSaturationSettings clean;
    clean.character = 0.0;
    for (double x : { -1.0, -0.5, -0.1, 0.0, 0.1, 0.5, 1.0 })
        assert(std::abs(analogTransfer(x, clean) - x) < 1.0e-12);

    // Gentle character should stay finite, preserve polarity around zero, and compress larger peaks.
    AnalogSaturationSettings gentle;
    gentle.character = 0.25;
    gentle.driveDb = 6.0;
    gentle.asymmetry = 0.0;
    const auto small = analogTransfer(0.05, gentle);
    const auto large = analogTransfer(1.0, gentle);
    assert(std::isfinite(small));
    assert(std::isfinite(large));
    assert(small > 0.0);
    assert(large > 0.0);
    assert(large < 1.0);

    // Asymmetry must create a measurable difference between positive/negative transfer magnitudes.
    gentle.asymmetry = 0.75;
    const auto positive = analogTransfer(0.6, gentle);
    const auto negative = analogTransfer(-0.6, gentle);
    assert(std::abs(std::abs(positive) - std::abs(negative)) > 1.0e-5);

    // DC blocker must reject a constant input over time without becoming unstable.
    DcBlocker blocker;
    blocker.prepare(48000.0, 7.0);
    float y = 0.0f;
    for (int i = 0; i < 48000 * 2; ++i)
        y = blocker.process(0, 0.25f);
    assert(std::isfinite(y));
    assert(std::abs(y) < 0.01f);

    blocker.reset();
    assert(std::abs(blocker.process(0, 0.0f)) < 1.0e-9f);
    return 0;
}
