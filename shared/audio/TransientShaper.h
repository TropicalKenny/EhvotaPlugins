#pragma once

#include <array>
#include <cstddef>

namespace ehvota::audio {

struct TransientShaperSettings {
    double attackPercent {};
    double sustainPercent {};
    double sensitivityPercent { 50.0 };
    double outputDb {};
};

class StereoTransientShaper {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setSettings(const TransientShaperSettings& settings) noexcept;

    // Stereo-linked control signal preserves image; call once per frame, then apply
    // the returned gain equally to L/R (or to a mono sample).
    [[nodiscard]] float gainForFrame(float left, float right) noexcept;
    [[nodiscard]] float lastTransientAmount() const noexcept { return lastTransientAmount_; }

private:
    double sampleRate_ { 44100.0 };
    double fastEnvelope_ {};
    double slowEnvelope_ {};
    double fastAttackCoeff_ {};
    double fastReleaseCoeff_ {};
    double slowAttackCoeff_ {};
    double slowReleaseCoeff_ {};
    TransientShaperSettings settings_;
    float lastTransientAmount_ {};
};

} // namespace ehvota::audio
