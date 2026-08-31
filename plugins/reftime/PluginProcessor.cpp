#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace ehvota::reftime {
namespace {

constexpr const char* divisionParam = "division";
constexpr const char* modifierParam = "modifier";
constexpr const char* outputModeParam = "outputMode";
constexpr const char* manualBpmParam = "manualBpm";

int divisionFromChoice(const int index) noexcept
{
    static constexpr int values[] { 1, 2, 4, 8, 16, 32, 64 };
    return values[juce::jlimit(0, 6, index)];
}

timing::NoteModifier modifierFromChoice(const int index) noexcept
{
    switch (index) {
    case 1: return timing::NoteModifier::dotted;
    case 2: return timing::NoteModifier::triplet;
    default: return timing::NoteModifier::straight;
    }
}

} // namespace

RefTimeAudioProcessor::RefTimeAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout RefTimeAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(divisionParam, 1), "Division",
        juce::StringArray { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64" }, 5));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(modifierParam, 1), "Modifier",
        juce::StringArray { "Straight", "Dotted", "Triplet" }, 1));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(outputModeParam, 1), "Output Mode",
        juce::StringArray { "Delayed Only", "Dry + Delayed", "Dry Only" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(manualBpmParam, 1), "Manual BPM",
        juce::NormalisableRange<float>(20.0f, 400.0f, 0.001f), 120.0f));
    return layout;
}

void RefTimeAudioProcessor::prepareToPlay(const double sampleRate, const int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    delayBufferSize_ = static_cast<int>(std::ceil(sampleRate_ * 16.0)) + samplesPerBlock + 8;
    delayBuffer_.assign(static_cast<std::size_t>(juce::jmax(1, getTotalNumOutputChannels())),
        std::vector<float>(static_cast<std::size_t>(delayBufferSize_), 0.0f));
    writePosition_ = 0;
    activeDelaySamples_ = 0.0;
    previousDelaySamples_ = 0.0;
    crossfadeSamplesTotal_ = juce::jmax(32, static_cast<int>(sampleRate_ * 0.02));
    crossfadeSamplesRemaining_ = 0;
    updateTimingTarget();
}

void RefTimeAudioProcessor::releaseResources()
{
    delayBuffer_.clear();
    delayBufferSize_ = 0;
}

bool RefTimeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void RefTimeAudioProcessor::updateTimingTarget()
{
    double bpm = static_cast<double>(parameters_.getRawParameterValue(manualBpmParam)->load());
    if (auto* playHead = getPlayHead()) {
        if (auto position = playHead->getPosition()) {
            if (const auto hostTempo = position->getBpm())
                bpm = *hostTempo;
        }
    }

    bpm = juce::jlimit(20.0, 400.0, bpm);
    hostBpm_.store(bpm, std::memory_order_relaxed);

    const auto divisionIndex = static_cast<int>(parameters_.getRawParameterValue(divisionParam)->load());
    const auto modifierIndex = static_cast<int>(parameters_.getRawParameterValue(modifierParam)->load());
    const auto timingValue = timing::calculateNoteTiming(
        bpm,
        divisionFromChoice(divisionIndex),
        modifierFromChoice(modifierIndex),
        sampleRate_);

    exactMilliseconds_.store(timingValue.exactMilliseconds, std::memory_order_relaxed);
    exactSamples_.store(timingValue.samples, std::memory_order_relaxed);

    const auto maximumDelay = static_cast<double>(juce::jmax(0, delayBufferSize_ - 4));
    const auto newDelay = juce::jlimit(0.0, maximumDelay, static_cast<double>(timingValue.samples));
    if (std::abs(newDelay - activeDelaySamples_) > 0.5) {
        previousDelaySamples_ = activeDelaySamples_;
        activeDelaySamples_ = newDelay;
        crossfadeSamplesRemaining_ = crossfadeSamplesTotal_;
    }
}

float RefTimeAudioProcessor::readDelaySample(const int channel, const double delaySamples) const noexcept
{
    if (delayBufferSize_ <= 1 || channel < 0 || channel >= static_cast<int>(delayBuffer_.size()))
        return 0.0f;

    double readPosition = static_cast<double>(writePosition_) - delaySamples;
    while (readPosition < 0.0)
        readPosition += static_cast<double>(delayBufferSize_);
    while (readPosition >= static_cast<double>(delayBufferSize_))
        readPosition -= static_cast<double>(delayBufferSize_);

    const auto indexA = static_cast<int>(readPosition);
    const auto indexB = (indexA + 1) % delayBufferSize_;
    const auto fraction = static_cast<float>(readPosition - static_cast<double>(indexA));
    const auto& channelBuffer = delayBuffer_[static_cast<std::size_t>(channel)];
    return channelBuffer[static_cast<std::size_t>(indexA)]
        + (channelBuffer[static_cast<std::size_t>(indexB)] - channelBuffer[static_cast<std::size_t>(indexA)]) * fraction;
}

void RefTimeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (delayBufferSize_ <= 0 || delayBuffer_.empty())
        return;

    updateTimingTarget();
    const auto mode = static_cast<int>(parameters_.getRawParameterValue(outputModeParam)->load());
    const auto channels = juce::jmin(buffer.getNumChannels(), static_cast<int>(delayBuffer_.size()));

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        const auto crossfadeAlpha = crossfadeSamplesRemaining_ > 0
            ? 1.0f - static_cast<float>(crossfadeSamplesRemaining_) / static_cast<float>(crossfadeSamplesTotal_)
            : 1.0f;

        for (int channel = 0; channel < channels; ++channel) {
            auto* output = buffer.getWritePointer(channel);
            const auto dry = output[sample];
            delayBuffer_[static_cast<std::size_t>(channel)][static_cast<std::size_t>(writePosition_)] = dry;

            const auto newTap = readDelaySample(channel, activeDelaySamples_);
            const auto oldTap = readDelaySample(channel, previousDelaySamples_);
            const auto delayed = crossfadeSamplesRemaining_ > 0
                ? juce::jmap(crossfadeAlpha, oldTap, newTap)
                : newTap;

            switch (mode) {
            case 1: output[sample] = dry + delayed; break;
            case 2: output[sample] = dry; break;
            default: output[sample] = delayed; break;
            }
        }

        writePosition_ = (writePosition_ + 1) % delayBufferSize_;
        if (crossfadeSamplesRemaining_ > 0)
            --crossfadeSamplesRemaining_;
    }
}

double RefTimeAudioProcessor::getTailLengthSeconds() const
{
    return exactMilliseconds_.load(std::memory_order_relaxed) / 1000.0;
}

juce::AudioProcessorEditor* RefTimeAudioProcessor::createEditor()
{
    return new RefTimeAudioProcessorEditor(*this);
}

void RefTimeAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = parameters_.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void RefTimeAudioProcessor::setStateInformation(const void* data, const int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters_.state.getType()))
            parameters_.replaceState(juce::ValueTree::fromXml(*xml));
}

} // namespace ehvota::reftime

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ehvota::reftime::RefTimeAudioProcessor();
}
