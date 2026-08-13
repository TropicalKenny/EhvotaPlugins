#include "TransientShaper.h"

#include <algorithm>
#include <cmath>

namespace ehvota::audio {
namespace {

double coefficientForMilliseconds(const double milliseconds, const double sampleRate) noexcept
{
    const auto seconds = std::max(milliseconds, 0.01) * 0.001;
    return std::exp(-1.0 / (seconds * std::max(sampleRate, 1.0)));
}

double followEnvelope(
    const double input,
    const double current,
    const double attackCoefficient,
    const double releaseCoefficient) noexcept
{
    const auto coefficient = input > current ? attackCoefficient : releaseCoefficient;
    return coefficient * current + (1.0 - coefficient) * input;
}

double dbToGain(const double db) noexcept
{
    return std::pow(10.0, db / 20.0);
}

} // namespace

void StereoTransientShaper::prepare(const double sampleRate) noexcept
{
    sampleRate_ = std::max(sampleRate, 1.0);
    fastAttackCoeff_ = coefficientForMilliseconds(0.35, sampleRate_);
    fastReleaseCoeff_ = coefficientForMilliseconds(24.0, sampleRate_);
    slowAttackCoeff_ = coefficientForMilliseconds(11.0, sampleRate_);
    slowReleaseCoeff_ = coefficientForMilliseconds(180.0, sampleRate_);
    reset();
}

void StereoTransientShaper::reset() noexcept
{
    fastEnvelope_ = 0.0;
    slowEnvelope_ = 0.0;
    lastTransientAmount_ = 0.0f;
}

void StereoTransientShaper::setSettings(const TransientShaperSettings& settings) noexcept
{
    settings_ = settings;
    settings_.attackPercent = std::clamp(settings_.attackPercent, -100.0, 100.0);
    settings_.sustainPercent = std::clamp(settings_.sustainPercent, -100.0, 100.0);
    settings_.sensitivityPercent = std::clamp(settings_.sensitivityPercent, 0.0, 100.0);
    settings_.outputDb = std::clamp(settings_.outputDb, -18.0, 18.0);
}

float StereoTransientShaper::gainForFrame(const float left, const float right) noexcept
{
    const auto detector = std::max(std::abs(static_cast<double>(left)), std::abs(static_cast<double>(right)));
    fastEnvelope_ = followEnvelope(detector, fastEnvelope_, fastAttackCoeff_, fastReleaseCoeff_);
    slowEnvelope_ = followEnvelope(detector, slowEnvelope_, slowAttackCoeff_, slowReleaseCoeff_);

    const auto sensitivity = settings_.sensitivityPercent * 0.01;
    const auto normalisationFloor = 0.25 * (1.0 - sensitivity) + 0.008;
    const auto denominator = slowEnvelope_ + normalisationFloor;
    const auto attackStrength = std::clamp((fastEnvelope_ - slowEnvelope_) / denominator, 0.0, 1.0);
    const auto sustainStrength = std::clamp((slowEnvelope_ - fastEnvelope_) / denominator, 0.0, 1.0);

    const auto attackDb = (settings_.attackPercent * 0.01) * attackStrength * 12.0;
    const auto sustainDb = (settings_.sustainPercent * 0.01) * sustainStrength * 9.0;
    const auto totalDb = std::clamp(attackDb + sustainDb + settings_.outputDb, -24.0, 24.0);
    lastTransientAmount_ = static_cast<float>(attackStrength - sustainStrength);
    return static_cast<float>(dbToGain(totalDb));
}

} // namespace ehvota::audio
