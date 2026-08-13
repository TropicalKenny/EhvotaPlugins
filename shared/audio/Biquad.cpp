#include "Biquad.h"

#include <algorithm>
#include <cmath>

namespace ehvota::audio {
namespace {

constexpr double pi = 3.14159265358979323846;

struct Common {
    double w0 {};
    double cosW0 {};
    double sinW0 {};
    double alpha {};
};

Common common(double sampleRate, double frequencyHz, double q) noexcept
{
    const auto safeRate = std::max(sampleRate, 1.0);
    const auto safeFrequency = std::clamp(frequencyHz, 1.0, safeRate * 0.49);
    const auto safeQ = std::max(q, 0.01);
    const auto w0 = 2.0 * pi * safeFrequency / safeRate;
    const auto sinW0 = std::sin(w0);
    return { w0, std::cos(w0), sinW0, sinW0 / (2.0 * safeQ) };
}

BiquadCoefficients normalise(
    double b0,
    double b1,
    double b2,
    double a0,
    double a1,
    double a2) noexcept
{
    if (std::abs(a0) < 1.0e-18)
        return {};
    const auto inverse = 1.0 / a0;
    return { b0 * inverse, b1 * inverse, b2 * inverse, a1 * inverse, a2 * inverse };
}

} // namespace

BiquadCoefficients makeLowPass(double sampleRate, double frequencyHz, double q) noexcept
{
    const auto c = common(sampleRate, frequencyHz, q);
    const auto b0 = (1.0 - c.cosW0) * 0.5;
    const auto b1 = 1.0 - c.cosW0;
    const auto b2 = b0;
    const auto a0 = 1.0 + c.alpha;
    const auto a1 = -2.0 * c.cosW0;
    const auto a2 = 1.0 - c.alpha;
    return normalise(b0, b1, b2, a0, a1, a2);
}

BiquadCoefficients makeHighPass(double sampleRate, double frequencyHz, double q) noexcept
{
    const auto c = common(sampleRate, frequencyHz, q);
    const auto b0 = (1.0 + c.cosW0) * 0.5;
    const auto b1 = -(1.0 + c.cosW0);
    const auto b2 = b0;
    const auto a0 = 1.0 + c.alpha;
    const auto a1 = -2.0 * c.cosW0;
    const auto a2 = 1.0 - c.alpha;
    return normalise(b0, b1, b2, a0, a1, a2);
}

BiquadCoefficients makePeaking(
    double sampleRate,
    double frequencyHz,
    double q,
    double gainDb) noexcept
{
    const auto c = common(sampleRate, frequencyHz, q);
    const auto a = std::pow(10.0, gainDb / 40.0);
    const auto b0 = 1.0 + c.alpha * a;
    const auto b1 = -2.0 * c.cosW0;
    const auto b2 = 1.0 - c.alpha * a;
    const auto a0 = 1.0 + c.alpha / a;
    const auto a1 = -2.0 * c.cosW0;
    const auto a2 = 1.0 - c.alpha / a;
    return normalise(b0, b1, b2, a0, a1, a2);
}

void StereoBiquad::reset() noexcept
{
    state_ = {};
}

float StereoBiquad::process(const std::size_t channel, const float input) noexcept
{
    const auto index = std::min<std::size_t>(channel, state_.size() - 1);
    auto& state = state_[index];
    const auto x = static_cast<double>(input);
    const auto y = coefficients_.b0 * x + state.z1;
    state.z1 = coefficients_.b1 * x - coefficients_.a1 * y + state.z2;
    state.z2 = coefficients_.b2 * x - coefficients_.a2 * y;
    return static_cast<float>(y);
}

} // namespace ehvota::audio
