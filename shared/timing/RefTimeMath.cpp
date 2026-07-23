#include "RefTimeMath.h"

#include <cmath>
#include <stdexcept>

namespace ehvota::timing {
namespace {

double modifierRatio(const NoteModifier modifier)
{
    switch (modifier) {
    case NoteModifier::straight:
        return 1.0;
    case NoteModifier::dotted:
        return 1.5;
    case NoteModifier::triplet:
        return 2.0 / 3.0;
    }

    throw std::invalid_argument("Unsupported note modifier.");
}

} // namespace

TimingValue calculateNoteTiming(
    const double beatsPerMinute,
    const int noteDenominator,
    const NoteModifier modifier,
    const double sampleRate)
{
    if (!std::isfinite(beatsPerMinute) || beatsPerMinute <= 0.0)
        throw std::invalid_argument("BPM must be finite and greater than zero.");
    if (noteDenominator <= 0)
        throw std::invalid_argument("Note denominator must be greater than zero.");
    if (!std::isfinite(sampleRate) || sampleRate <= 0.0)
        throw std::invalid_argument("Sample rate must be finite and greater than zero.");

    constexpr double millisecondsPerMinute = 60'000.0;
    const auto quarterNoteMilliseconds = millisecondsPerMinute / beatsPerMinute;
    const auto denominatorRatio = 4.0 / static_cast<double>(noteDenominator);
    const auto milliseconds = quarterNoteMilliseconds * denominatorRatio * modifierRatio(modifier);

    return {
        milliseconds,
        static_cast<std::int64_t>(std::llround(milliseconds * sampleRate / 1'000.0)),
        1'000.0 / milliseconds
    };
}

} // namespace ehvota::timing
