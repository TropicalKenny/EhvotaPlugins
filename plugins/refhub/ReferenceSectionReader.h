#pragma once

#include <juce_core/juce_core.h>

#include <cstddef>
#include <string>
#include <vector>

namespace ehvota::refhub {

struct ReferenceSectionAudio {
    bool valid {};
    std::vector<float> left;
    std::vector<float> right;
    double sampleRate {};
    double startSeconds {};
    double durationSeconds {};
    std::string error;
};

// Offline-only helper. It owns a separate reader/transport and may be called from a worker thread.
// AudioTransportSource performs source sample-rate correction into the requested project rate.
[[nodiscard]] ReferenceSectionAudio readReferenceSection(
    const juce::File& file,
    double startSeconds,
    double targetSampleRate,
    std::size_t targetSamples);

} // namespace ehvota::refhub
