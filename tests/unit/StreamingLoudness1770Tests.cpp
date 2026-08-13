#include "shared/analysis/Loudness1770.h"
#include "shared/analysis/StreamingLoudness1770.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

namespace {
constexpr double pi = 3.14159265358979323846;
}

int main()
{
    using namespace ehvota::analysis;
    constexpr double sampleRate = 48000.0;
    constexpr double seconds = 8.0;
    const auto samples = static_cast<std::size_t>(sampleRate * seconds);

    std::vector<float> left(samples);
    std::vector<float> right(samples);
    for (std::size_t i = 0; i < samples; ++i) {
        const auto t = static_cast<double>(i) / sampleRate;
        const auto envelope = t < 1.0 ? 0.15 : (t < 6.0 ? 0.30 : 0.08);
        left[i] = static_cast<float>(envelope * std::sin(2.0 * pi * 440.0 * t));
        right[i] = static_cast<float>(envelope * 0.9 * std::sin(2.0 * pi * 440.0 * t + 0.2));
    }

    const Loudness1770 offline(sampleRate);
    const auto expected = offline.measureStereo(left, right);
    assert(expected.valid);

    StreamingLoudness1770 streaming;
    assert(streaming.prepare(sampleRate, 60.0));
    assert(streaming.begin());
    std::size_t offset = 0;
    constexpr std::size_t chunk = 257;
    while (offset < samples) {
        const auto count = std::min(chunk, samples - offset);
        streaming.processStereo(
            std::span<const float>(left.data() + offset, count),
            std::span<const float>(right.data() + offset, count));
        offset += count;
    }
    const auto actual = streaming.finish();
    assert(actual.state == StreamingLoudnessState::complete);
    assert(actual.loudness.valid);
    assert(actual.samplesProcessed == samples);
    assert(std::abs(actual.loudness.integratedLufs - expected.integratedLufs) < 1.0e-8);
    assert(actual.loudness.totalBlocks == expected.totalBlocks);
    assert(actual.loudness.gatedBlocks == expected.gatedBlocks);

    StreamingLoudness1770 limited;
    assert(limited.prepare(sampleRate, 1.0));
    assert(limited.begin());
    limited.processStereo(left, right);
    const auto limitedResult = limited.finish();
    assert(limitedResult.state == StreamingLoudnessState::capacityExceeded);
    assert(limitedResult.samplesProcessed == limitedResult.maximumSamples);

    StreamingLoudness1770 tooShort;
    assert(tooShort.prepare(sampleRate, 5.0));
    assert(tooShort.begin());
    tooShort.processStereo(
        std::span<const float>(left.data(), 1000),
        std::span<const float>(right.data(), 1000));
    const auto shortResult = tooShort.finish();
    assert(shortResult.state == StreamingLoudnessState::complete);
    assert(!shortResult.loudness.valid);

    return 0;
}
