#include "shared/audio/TransientShaper.h"

#include <cassert>
#include <cmath>

int main()
{
    using namespace ehvota::audio;

    StereoTransientShaper shaper;
    shaper.prepare(48000.0);
    shaper.setSettings({});

    // Neutral settings must produce unity gain.
    for (int i = 0; i < 1000; ++i)
        assert(std::abs(shaper.gainForFrame(0.25f, -0.25f) - 1.0f) < 1.0e-6f);

    // Positive attack should increase the gain of a sudden onset.
    shaper.reset();
    TransientShaperSettings attack;
    attack.attackPercent = 100.0;
    attack.sensitivityPercent = 100.0;
    shaper.setSettings(attack);
    for (int i = 0; i < 2000; ++i)
        shaper.gainForFrame(0.0f, 0.0f);
    const auto onsetGain = shaper.gainForFrame(0.9f, 0.9f);
    assert(std::isfinite(onsetGain));
    assert(onsetGain > 1.0f);

    // Negative attack should attenuate the same onset.
    shaper.reset();
    attack.attackPercent = -100.0;
    shaper.setSettings(attack);
    for (int i = 0; i < 2000; ++i)
        shaper.gainForFrame(0.0f, 0.0f);
    const auto softenedGain = shaper.gainForFrame(0.9f, 0.9f);
    assert(softenedGain < 1.0f);

    // Stereo-linked detector must return one gain for both channels and stay finite.
    shaper.reset();
    attack.attackPercent = 35.0;
    attack.sustainPercent = 20.0;
    shaper.setSettings(attack);
    for (int i = 0; i < 48000; ++i) {
        const auto gain = shaper.gainForFrame(i == 500 ? 1.0f : 0.1f, 0.02f);
        assert(std::isfinite(gain));
        assert(gain > 0.0f);
    }

    return 0;
}
