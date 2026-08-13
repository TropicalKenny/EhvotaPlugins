#include "AnalogSaturation.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ehvota::audio {
namespace {

double dbToGain(const double db) noexcept
{
    return std::pow(10.0, db / 20.0);
}

} // namespace

double analogTransfer(const double input, const AnalogSaturationSettings& settings) noexcept
{
    const auto character = std::clamp(settings.character, 0.0, 1.0);
    const auto asymmetry = std::clamp(settings.asymmetry, -1.0, 1.0);
    const auto driveDb = std::clamp(settings.driveDb, 0.0, 24.0);
    const auto outputGain = dbToGain(std::clamp(settings.outputDb, -24.0, 24.0));

    if (character <= 0.0)
        return input * outputGain;

    // k controls curvature rather than simple pre-gain. This keeps the small-signal
    // response close to unity while progressively compressing larger excursions.
    const auto baseK = 1.0 + driveDb * 0.34;
    const auto polarity = input >= 0.0 ? 1.0 : -1.0;
    const auto asymmetricScale = 1.0 + polarity * asymmetry * 0.22;
    const auto k = std::max(0.25, baseK * asymmetricScale);

    // Calibrate the nonlinear path around a moderate musical operating level so
    // enabling gentle character does not win a comparison merely by getting louder.
    constexpr double calibrationAmplitude = 0.18;
    const auto calibration = std::tanh(k * calibrationAmplitude) / (k * calibrationAmplitude);
    const auto wet = std::tanh(k * input) / (k * std::max(calibration, 1.0e-9));
    const auto blended = input + (wet - input) * character;
    return blended * outputGain;
}

void DcBlocker::prepare(const double sampleRate, const double cutoffHz) noexcept
{
    if (sampleRate <= 0.0) {
        coefficient_ = 0.999;
        return;
    }

    const auto cutoff = std::clamp(cutoffHz, 1.0, sampleRate * 0.05);
    coefficient_ = std::exp(-2.0 * std::numbers::pi * cutoff / sampleRate);
}

void DcBlocker::reset() noexcept
{
    previousInput_.fill(0.0);
    previousOutput_.fill(0.0);
}

float DcBlocker::process(const std::size_t channel, const float sample) noexcept
{
    const auto index = std::min<std::size_t>(channel, previousInput_.size() - 1);
    const auto x = static_cast<double>(sample);
    const auto y = x - previousInput_[index] + coefficient_ * previousOutput_[index];
    previousInput_[index] = x;
    previousOutput_[index] = y;
    return static_cast<float>(y);
}

} // namespace ehvota::audio
