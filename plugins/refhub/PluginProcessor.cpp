#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace ehvota::refhub {

RefHubAudioProcessor::RefHubAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void RefHubAudioProcessor::prepareToPlay(const double, const int)
{
}

void RefHubAudioProcessor::releaseResources()
{
}

bool RefHubAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void RefHubAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    float peak = 0.0f;
    double sumSquares = 0.0;
    std::uint64_t count = 0;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        const auto* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            const auto value = data[sample];
            peak = juce::jmax(peak, std::abs(value));
            sumSquares += static_cast<double>(value) * static_cast<double>(value);
            ++count;
        }
    }

    const auto rms = count > 0
        ? static_cast<float>(std::sqrt(sumSquares / static_cast<double>(count)))
        : 0.0f;
    livePeakDb_.store(juce::Decibels::gainToDecibels(peak, -100.0f), std::memory_order_relaxed);
    liveRmsDb_.store(juce::Decibels::gainToDecibels(rms, -100.0f), std::memory_order_relaxed);

    // Version 1 foundation: RefHub's live input path is measurement-only.
    // Reference audition and level-match gain live in dedicated monitoring paths and must never
    // alter the project signal that reaches the host output/export path.
}

juce::AudioProcessorEditor* RefHubAudioProcessor::createEditor()
{
    return new RefHubAudioProcessorEditor(*this);
}

void RefHubAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    juce::MemoryOutputStream stream(destination, true);
    stream.writeInt(1);
}

void RefHubAudioProcessor::setStateInformation(const void*, const int)
{
}

} // namespace ehvota::refhub

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ehvota::refhub::RefHubAudioProcessor();
}
