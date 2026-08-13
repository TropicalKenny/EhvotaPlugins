#include "shared/audio/Biquad.h"

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
} // namespace

int main()
{
    constexpr double sampleRate = 48000.0;

    ehvota::audio::StereoBiquad neutralPeak;
    neutralPeak.setCoefficients(ehvota::audio::makePeaking(sampleRate, 1000.0, 0.8, 0.0));
    double maxNeutralError = 0.0;
    for (int i = 0; i < 4096; ++i) {
        const auto input = static_cast<float>(0.5 * std::sin(2.0 * 3.14159265358979323846 * 1000.0 * i / sampleRate));
        const auto output = neutralPeak.process(0, input);
        maxNeutralError = std::max(maxNeutralError, std::abs(static_cast<double>(output - input)));
    }
    require(maxNeutralError < 1.0e-5, "0 dB peaking coefficients should behave as a numerical identity");

    ehvota::audio::StereoBiquad highPass;
    highPass.setCoefficients(ehvota::audio::makeHighPass(sampleRate, 100.0, 0.7071067811865476));
    double lateDcMagnitude = 0.0;
    for (int i = 0; i < 48000; ++i) {
        const auto output = highPass.process(0, 1.0f);
        if (i > 47000)
            lateDcMagnitude = std::max(lateDcMagnitude, std::abs(static_cast<double>(output)));
    }
    require(lateDcMagnitude < 1.0e-3, "high-pass should strongly reject settled DC");

    const auto coefficients = ehvota::audio::makeLowPass(sampleRate, 20000.0, 0.7071067811865476);
    require(std::isfinite(coefficients.b0) && std::isfinite(coefficients.a2),
        "coefficient generation must remain finite near the top of the supported range");

    std::cout << "Biquad tests passed\n";
    return 0;
}
