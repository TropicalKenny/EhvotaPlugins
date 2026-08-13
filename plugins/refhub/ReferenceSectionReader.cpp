#include "ReferenceSectionReader.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

namespace ehvota::refhub {

ReferenceSectionAudio readReferenceSection(
    const juce::File& file,
    const double startSeconds,
    const double targetSampleRate,
    const std::size_t targetSamples)
{
    ReferenceSectionAudio result;
    result.startSeconds = startSeconds;
    result.sampleRate = targetSampleRate;

    if (!file.existsAsFile()) {
        result.error = "Reference file no longer exists.";
        return result;
    }
    if (!(startSeconds >= 0.0) || !(targetSampleRate >= 8000.0)
        || !std::isfinite(startSeconds) || !std::isfinite(targetSampleRate)
        || targetSamples == 0 || targetSamples > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        result.error = "Reference section request is invalid.";
        return result;
    }

    result.durationSeconds = static_cast<double>(targetSamples) / targetSampleRate;
    if (!(result.durationSeconds > 0.0) || result.durationSeconds > 60.001) {
        result.error = "Reference section exceeds the bounded 60 second analysis window.";
        return result;
    }

    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    auto reader = std::unique_ptr<juce::AudioFormatReader>(formats.createReaderFor(file));
    if (!reader || !(reader->sampleRate > 0.0)) {
        result.error = "Reference format could not be read.";
        return result;
    }

    const auto sourceRate = reader->sampleRate;
    const auto sourceChannels = reader->numChannels;
    const auto sourceDuration = static_cast<double>(reader->lengthInSamples) / sourceRate;
    if (startSeconds + result.durationSeconds > sourceDuration + 0.001) {
        result.error = "Reference section extends beyond the authoritative full mix.";
        return result;
    }

    auto readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);
    juce::AudioTransportSource transport;
    transport.setSource(readerSource.get(), 0, nullptr, sourceRate, 2);
    transport.prepareToPlay(4096, targetSampleRate);
    transport.setPosition(startSeconds);
    transport.start();

    const auto outputSamples = static_cast<int>(targetSamples);
    juce::AudioBuffer<float> output(2, outputSamples);
    output.clear();
    int write = 0;
    while (write < outputSamples) {
        const auto blockSamples = std::min(4096, outputSamples - write);
        juce::AudioSourceChannelInfo info(&output, write, blockSamples);
        transport.getNextAudioBlock(info);
        write += blockSamples;
    }

    transport.stop();
    transport.releaseResources();
    transport.setSource(nullptr);

    result.left.assign(output.getReadPointer(0), output.getReadPointer(0) + outputSamples);
    if (sourceChannels > 1)
        result.right.assign(output.getReadPointer(1), output.getReadPointer(1) + outputSamples);
    else
        result.right = result.left;

    result.valid = result.left.size() == targetSamples && result.right.size() == targetSamples;
    if (!result.valid)
        result.error = "Reference section read returned an incomplete buffer.";
    return result;
}

} // namespace ehvota::refhub
