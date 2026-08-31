#include "shared/analysis/LevelMatch.h"
#include "shared/analysis/Loudness1770.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

constexpr double pi = 3.14159265358979323846;

std::vector<float> sine(double sampleRate, double seconds, double amplitude)
{
    const auto count = static_cast<std::size_t>(sampleRate * seconds);
    std::vector<float> result(count);
    for (std::size_t i = 0; i < count; ++i)
        result[i] = static_cast<float>(amplitude * std::sin(2.0 * pi * 1000.0 * static_cast<double>(i) / sampleRate));
    return result;
}

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
    ehvota::analysis::Loudness1770 meter(sampleRate);

    const auto quiet = sine(sampleRate, 2.0, 0.10);
    const auto loud = sine(sampleRate, 2.0, 0.20);
    const auto quietResult = meter.measureStereo(quiet, quiet);
    const auto loudResult = meter.measureStereo(loud, loud);

    require(quietResult.valid, "quiet sine should produce valid gated loudness");
    require(loudResult.valid, "loud sine should produce valid gated loudness");
    const double delta = loudResult.integratedLufs - quietResult.integratedLufs;
    require(std::abs(delta - 6.0206) < 0.08, "doubling amplitude should raise loudness by about 6.02 LU");

    const auto decision = ehvota::analysis::calculateReferenceMonitorTrim(quietResult, loudResult);
    require(decision.valid, "level match should resolve for two valid captures");
    require(std::abs(decision.referenceMonitorTrimDb + 6.0206) < 0.08,
        "a 6 dB louder reference should receive approximately -6 dB monitor trim");

    std::vector<float> silence(static_cast<std::size_t>(sampleRate), 0.0f);
    const auto silenceResult = meter.measureStereo(silence, silence);
    require(!silenceResult.valid, "silence should not be treated as stable level-match evidence");

    const auto invalidDecision = ehvota::analysis::calculateReferenceMonitorTrim(silenceResult, loudResult);
    require(!invalidDecision.valid, "invalid loudness evidence must not create monitor trim");

    std::cout << "Loudness1770 tests passed\n";
    return 0;
}
