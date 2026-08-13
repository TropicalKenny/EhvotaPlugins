#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace ehvota::reftap {

RefTapAudioProcessor::RefTapAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    startTimerHz(10);
}

RefTapAudioProcessor::~RefTapAudioProcessor()
{
    stopTimer();
}

void RefTapAudioProcessor::prepareToPlay(const double sampleRate, const int)
{
    sampleRate_ = sampleRate;
    sampleCounter_.store(0, std::memory_order_relaxed);
}

void RefTapAudioProcessor::releaseResources()
{
}

bool RefTapAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;

    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void RefTapAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto channels = buffer.getNumChannels();
    const auto samples = buffer.getNumSamples();
    if (channels <= 0 || samples <= 0)
        return;

    float maxMagnitude = 0.0f;
    double sumSquares = 0.0;
    std::uint64_t sampleCount = 0;

    for (int channel = 0; channel < channels; ++channel) {
        const auto* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < samples; ++sample) {
            const auto value = data[sample];
            maxMagnitude = juce::jmax(maxMagnitude, std::abs(value));
            sumSquares += static_cast<double>(value) * static_cast<double>(value);
            ++sampleCount;
        }
    }

    const auto rms = sampleCount > 0
        ? static_cast<float>(std::sqrt(sumSquares / static_cast<double>(sampleCount)))
        : 0.0f;

    const auto peakDb = juce::Decibels::gainToDecibels(maxMagnitude, -100.0f);
    const auto rmsDb = juce::Decibels::gainToDecibels(rms, -100.0f);
    peakDb_.store(peakDb, std::memory_order_relaxed);
    rmsDb_.store(rmsDb, std::memory_order_relaxed);

    float correlation = 1.0f;
    if (channels >= 2) {
        const auto* left = buffer.getReadPointer(0);
        const auto* right = buffer.getReadPointer(1);
        double lr = 0.0;
        double ll = 0.0;
        double rr = 0.0;

        for (int sample = 0; sample < samples; ++sample) {
            const auto l = static_cast<double>(left[sample]);
            const auto r = static_cast<double>(right[sample]);
            lr += l * r;
            ll += l * l;
            rr += r * r;
        }

        const auto denominator = std::sqrt(ll * rr);
        const auto value = denominator > 1.0e-12 ? lr / denominator : 1.0;
        correlation = static_cast<float>(juce::jlimit(-1.0, 1.0, value));
    }
    correlation_.store(correlation, std::memory_order_relaxed);

    sampleCounter_.fetch_add(static_cast<std::uint64_t>(samples), std::memory_order_relaxed);
    crestDb_.store(maxMagnitude > 0.0f && rms > 0.0f ? peakDb - rmsDb : 0.0f, std::memory_order_relaxed);

    // RefTap is deliberately bit-transparent. Cross-plugin publication happens from timerCallback(),
    // never from the realtime audio callback.
}

void RefTapAudioProcessor::timerCallback()
{
    refLink_.publish({
        peakDb_.load(std::memory_order_relaxed),
        rmsDb_.load(std::memory_order_relaxed),
        correlation_.load(std::memory_order_relaxed),
        crestDb_.load(std::memory_order_relaxed),
        sampleCounter_.load(std::memory_order_relaxed)
    });
}

juce::AudioProcessorEditor* RefTapAudioProcessor::createEditor()
{
    return new RefTapAudioProcessorEditor(*this);
}

void RefTapAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    juce::MemoryOutputStream stream(destination, true);
    stream.writeInt(2);
}

void RefTapAudioProcessor::setStateInformation(const void*, const int)
{
}

} // namespace ehvota::reftap

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ehvota::reftap::RefTapAudioProcessor();
}
