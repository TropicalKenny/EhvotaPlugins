#pragma once

#include <cstdint>

namespace ehvota::timing {

enum class NoteModifier {
    straight,
    dotted,
    triplet
};

struct TimingValue {
    double exactMilliseconds {};
    std::int64_t samples {};
    double equivalentHertz {};
};

[[nodiscard]] TimingValue calculateNoteTiming(
    double beatsPerMinute,
    int noteDenominator,
    NoteModifier modifier,
    double sampleRate);

} // namespace ehvota::timing
