#pragma once

#include <array>
#include <cstddef>

namespace ehvota::audio {

struct AnalogSaturationSettings {
    double driveDb { 0.0 };
    double asymmetry { 0.0 };   // -1..1. Adds a controlled even-harmonic component.
    double character { 0.0 };   // 0 clean/linear, 1 fully nonlinear.
    double outputDb { 0.0 };
};

[[nodiscard]] double analogTransfer(double input, const AnalogSaturationSettings& settings) noexcept;

class DcBlocker {
public:
    void prepare(double sampleRate, double cutoffHz = 7.0) noexcept;
    void reset() noexcept;
    [[nodiscard]] float process(std::size_t channel, float sample) noexcept;

private:
    double coefficient_ { 0.999 };
    std::array<double, 2> previousInput_ {};
    std::array<double, 2> previousOutput_ {};
};

} // namespace ehvota::audio
