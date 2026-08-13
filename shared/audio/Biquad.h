#pragma once

#include <array>
#include <cstddef>

namespace ehvota::audio {

struct BiquadCoefficients {
    double b0 { 1.0 };
    double b1 {};
    double b2 {};
    double a1 {};
    double a2 {};
};

[[nodiscard]] BiquadCoefficients makeLowPass(double sampleRate, double frequencyHz, double q) noexcept;
[[nodiscard]] BiquadCoefficients makeHighPass(double sampleRate, double frequencyHz, double q) noexcept;
[[nodiscard]] BiquadCoefficients makePeaking(
    double sampleRate,
    double frequencyHz,
    double q,
    double gainDb) noexcept;

class StereoBiquad {
public:
    void reset() noexcept;
    void setCoefficients(const BiquadCoefficients& coefficients) noexcept { coefficients_ = coefficients; }
    [[nodiscard]] const BiquadCoefficients& coefficients() const noexcept { return coefficients_; }
    float process(std::size_t channel, float input) noexcept;

private:
    struct State {
        double z1 {};
        double z2 {};
    };

    BiquadCoefficients coefficients_;
    std::array<State, 2> state_ {};
};

} // namespace ehvota::audio
