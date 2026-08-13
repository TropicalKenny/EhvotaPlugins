#include "ReferenceWholeTrackLoudness.h"

#include "shared/analysis/StreamingLoudness1770.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <span>

namespace ehvota::refhub {

ReferenceWholeTrackLoudness measureReferenceWholeTrackLoudness(
    const juce::File& file,
    const std::stop_token stopToken)
{
    ReferenceWholeTrackLoudness result;
    if (!file.existsAsFile()) {
        result.error = "Reference file does not exist.";
        return result;
    }

    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    auto reader = std::unique_ptr<juce::AudioFormatReader>(formats.createReaderFor(file));
    if (!reader || !(reader->sampleRate >= 8000.0) || reader->lengthInSamples <= 0) {
        result.error = "Reference format or metadata is invalid.";
        return result;
    }

    result.sampleRate = reader->sampleRate;
    result.sampleCount = static_cast<std::uint64_t>(reader->lengthInSamples);
    const auto durationSeconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    if (!std::isfinite(durationSeconds) || durationSeconds <= 0.0 || durationSeconds > 14400.0) {
        result.error = "Reference duration is outside the supported whole-track range.";
        return result;
    }

    ehvota::analysis::StreamingLoudness1770 meter;
    const auto meterCapacitySeconds = std::min(14400.0, std::max(0.5, durationSeconds + 1.0));
    if (!meter.prepare(reader->sampleRate, meterCapacitySeconds) || !meter.begin()) {
        result.error = "Could not prepare whole-track loudness measurement.";
        return result;
    }

    constexpr int chunkSamples = 16384;
    juce::AudioBuffer<float> chunk(2, chunkSamples);
    juce::int64 position = 0;
    while (position < reader->lengthInSamples) {
        if (stopToken.stop_requested()) {
            result.error = "Reference whole-track measurement was cancelled.";
            return result;
        }

        const auto remaining = reader->lengthInSamples - position;
        const auto count = static_cast<int>(std::min<juce::int64>(remaining, chunkSamples));
        chunk.clear();
        if (!reader->read(&chunk, 0, count, position, true, reader->numChannels > 1)) {
            result.error = "Reference audio could not be read during whole-track measurement.";
            return result;
        }

        const auto left = std::span<const float>(chunk.getReadPointer(0), static_cast<std::size_t>(count));
        const auto right = reader->numChannels > 1
            ? std::span<const float>(chunk.getReadPointer(1), static_cast<std::size_t>(count))
            : left;
        meter.processStereo(left, right);
        if (meter.state() == ehvota::analysis::StreamingLoudnessState::capacityExceeded) {
            result.error = "Reference exceeded the prepared whole-track measurement capacity.";
            return result;
        }
        position += count;
    }

    const auto measured = meter.finish();
    if (measured.state != ehvota::analysis::StreamingLoudnessState::complete || !measured.loudness.valid) {
        result.error = "Reference does not contain enough valid programme material for integrated loudness.";
        return result;
    }

    result.loudness = measured.loudness;
    result.valid = true;
    return result;
}

} // namespace ehvota::refhub
