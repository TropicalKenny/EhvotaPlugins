#pragma once

#include "shared/analysis/Loudness1770.h"

#include <juce_core/juce_core.h>

#include <cstdint>
#include <stop_token>
#include <string>

namespace ehvota::refhub {

struct ReferenceWholeTrackLoudness {
    bool valid {};
    ehvota::analysis::LoudnessResult loudness;
    double sampleRate {};
    std::uint64_t sampleCount {};
    std::string error;
};

// Worker-thread utility. Reads the file in bounded chunks and never holds the full
// reference in memory. The caller owns cancellation via stop_token.
[[nodiscard]] ReferenceWholeTrackLoudness measureReferenceWholeTrackLoudness(
    const juce::File& file,
    std::stop_token stopToken);

} // namespace ehvota::refhub
