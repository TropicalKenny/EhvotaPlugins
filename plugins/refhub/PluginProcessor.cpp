#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>
#include <span>

namespace ehvota::refhub {

RefHubAudioProcessor::RefHubAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    formatManager_.registerBasicFormats();
}

void RefHubAudioProcessor::prepareToPlay(const double sampleRate, const int samplesPerBlock)
{
    referenceTransport_.prepareToPlay(samplesPerBlock, sampleRate);
    // Section capture is intentionally bounded. Whole-track analysis uses the offline/reference
    // analysis path rather than reserving an entire song in realtime plugin memory.
    mixCapture_.prepare(sampleRate, 60.0);
}

void RefHubAudioProcessor::releaseResources()
{
    mixCapture_.cancel();
    referenceTransport_.releaseResources();
}

bool RefHubAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

bool RefHubAudioProcessor::loadReferenceFile(const juce::File& file)
{
    stopReferenceAudition();
    referenceTransport_.setSource(nullptr);
    referenceReaderSource_.reset();
    referenceLoaded_.store(false, std::memory_order_release);

    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager_.createReaderFor(file));
    if (!reader)
        return false;

    const auto sourceRate = reader->sampleRate;
    referenceReaderSource_ = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);
    referenceTransport_.setSource(referenceReaderSource_.get(), 0, nullptr, sourceRate, 2);
    referenceTransport_.setPosition(0.0);
    referenceLoaded_.store(true, std::memory_order_release);
    return true;
}

void RefHubAudioProcessor::unloadReferenceFile()
{
    stopReferenceAudition();
    referenceTransport_.setSource(nullptr);
    referenceReaderSource_.reset();
    referenceLoaded_.store(false, std::memory_order_release);
}

void RefHubAudioProcessor::startReferenceAudition()
{
    if (!hasReferenceLoaded())
        return;

    if (referenceTransport_.hasStreamFinished())
        referenceTransport_.setPosition(0.0);
    referenceTransport_.start();
    referenceAudition_.store(true, std::memory_order_release);
}

void RefHubAudioProcessor::stopReferenceAudition()
{
    referenceAudition_.store(false, std::memory_order_release);
    referenceTransport_.stop();
}

void RefHubAudioProcessor::setReferenceMonitorTrimDb(const float trimDb) noexcept
{
    referenceTrimDb_.store(juce::jlimit(-24.0f, 24.0f, trimDb), std::memory_order_relaxed);
}

void RefHubAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    float peak = 0.0f;
    double sumSquares = 0.0;
    std::uint64_t count = 0;

    // Measure and capture the project signal before any monitoring-only reference substitution.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const auto* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            const auto value = data[sample];
            peak = juce::jmax(peak, std::abs(value));
            sumSquares += static_cast<double>(value) * static_cast<double>(value);
            ++count;
        }
    }

    if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0) {
        const auto left = std::span<const float>(
            buffer.getReadPointer(0), static_cast<std::size_t>(buffer.getNumSamples()));
        const auto right = buffer.getNumChannels() > 1
            ? std::span<const float>(buffer.getReadPointer(1), static_cast<std::size_t>(buffer.getNumSamples()))
            : left;
        mixCapture_.push(left, right);
    }

    const auto rms = count > 0
        ? static_cast<float>(std::sqrt(sumSquares / static_cast<double>(count)))
        : 0.0f;
    livePeakDb_.store(juce::Decibels::gainToDecibels(peak, -100.0f), std::memory_order_relaxed);
    liveRmsDb_.store(juce::Decibels::gainToDecibels(rms, -100.0f), std::memory_order_relaxed);

    const bool offlineRenderProtected = renderSafe_.load(std::memory_order_acquire) && isNonRealtime();
    const bool shouldAuditionReference = referenceAudition_.load(std::memory_order_acquire)
        && referenceLoaded_.load(std::memory_order_acquire)
        && !offlineRenderProtected;

    if (shouldAuditionReference) {
        juce::AudioSourceChannelInfo info(&buffer, 0, buffer.getNumSamples());
        referenceTransport_.getNextAudioBlock(info);
        buffer.applyGain(juce::Decibels::decibelsToGain(referenceTrimDb_.load(std::memory_order_relaxed)));
        if (referenceTransport_.hasStreamFinished())
            referenceAudition_.store(false, std::memory_order_release);
    }

    // Render Safe is deliberately one-way: when the host declares an offline/non-realtime render,
    // reference audition is suppressed and the untouched project signal remains at the output.
}

juce::AudioProcessorEditor* RefHubAudioProcessor::createEditor()
{
    return new RefHubAudioProcessorEditor(*this);
}

void RefHubAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    juce::MemoryOutputStream stream(destination, true);
    stream.writeInt(2);
    stream.writeFloat(referenceTrimDb_.load(std::memory_order_relaxed));
    stream.writeBool(renderSafe_.load(std::memory_order_acquire));
}

void RefHubAudioProcessor::setStateInformation(const void* data, const int size)
{
    juce::MemoryInputStream stream(data, static_cast<std::size_t>(size), false);
    const auto version = stream.readInt();
    if (version >= 2) {
        setReferenceMonitorTrimDb(stream.readFloat());
        setRenderSafe(stream.readBool());
    }
}

} // namespace ehvota::refhub

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ehvota::refhub::RefHubAudioProcessor();
}
