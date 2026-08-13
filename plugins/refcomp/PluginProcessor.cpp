#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace ehvota::refcomp {
namespace {

float value(juce::AudioProcessorValueTreeState& state, const char* id)
{
    return state.getRawParameterValue(id)->load();
}

float computeGainReductionDb(const float inputDb, const float threshold, const float ratio, const float knee)
{
    if (knee <= 0.0f) {
        if (inputDb <= threshold)
            return 0.0f;
        const auto outputDb = threshold + (inputDb - threshold) / ratio;
        return outputDb - inputDb;
    }

    const auto lower = threshold - knee * 0.5f;
    const auto upper = threshold + knee * 0.5f;
    if (inputDb <= lower)
        return 0.0f;
    if (inputDb >= upper) {
        const auto outputDb = threshold + (inputDb - threshold) / ratio;
        return outputDb - inputDb;
    }

    const auto x = inputDb - lower;
    const auto slope = (1.0f / ratio) - 1.0f;
    return slope * x * x / (2.0f * knee);
}

} // namespace

RefCompAudioProcessor::RefCompAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout RefCompAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("threshold", 1), "Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -18.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("ratio", 1), "Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.01f, 0.45f), 4.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("attack", 1), "Attack", juce::NormalisableRange<float>(0.05f, 200.0f, 0.01f, 0.35f), 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("release", 1), "Release", juce::NormalisableRange<float>(5.0f, 2000.0f, 0.01f, 0.35f), 180.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("knee", 1), "Knee", juce::NormalisableRange<float>(0.0f, 24.0f, 0.01f), 6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("makeup", 1), "Makeup", juce::NormalisableRange<float>(-12.0f, 18.0f, 0.01f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("mix", 1), "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 100.0f));
    return layout;
}

void RefCompAudioProcessor::prepareToPlay(const double sampleRate, const int)
{
    sampleRate_ = sampleRate;
    envelopeDb_ = -120.0f;
    gainReductionDb_.store(0.0f, std::memory_order_relaxed);
}

void RefCompAudioProcessor::releaseResources()
{
}

bool RefCompAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void RefCompAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto threshold = value(parameters_, "threshold");
    const auto ratio = value(parameters_, "ratio");
    const auto attackMs = value(parameters_, "attack");
    const auto releaseMs = value(parameters_, "release");
    const auto kneeDb = value(parameters_, "knee");
    const auto makeupGain = juce::Decibels::decibelsToGain(value(parameters_, "makeup"));
    const auto wet = value(parameters_, "mix") * 0.01f;
    const auto dryMix = 1.0f - wet;

    const auto attackCoeff = std::exp(-1.0 / (0.001 * static_cast<double>(attackMs) * sampleRate_));
    const auto releaseCoeff = std::exp(-1.0 / (0.001 * static_cast<double>(releaseMs) * sampleRate_));

    float minimumGainReduction = 0.0f;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        float detector = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            detector = juce::jmax(detector, std::abs(buffer.getSample(channel, sample)));

        const auto detectorDb = juce::Decibels::gainToDecibels(detector, -120.0f);
        const auto coefficient = detectorDb > envelopeDb_ ? attackCoeff : releaseCoeff;
        envelopeDb_ = static_cast<float>(coefficient * envelopeDb_ + (1.0 - coefficient) * detectorDb);

        const auto reductionDb = computeGainReductionDb(envelopeDb_, threshold, ratio, kneeDb);
        minimumGainReduction = juce::jmin(minimumGainReduction, reductionDb);
        const auto compressedGain = juce::Decibels::decibelsToGain(reductionDb) * makeupGain;

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            const auto dry = buffer.getSample(channel, sample);
            const auto compressed = dry * compressedGain;
            buffer.setSample(channel, sample, dry * dryMix + compressed * wet);
        }
    }

    gainReductionDb_.store(-minimumGainReduction, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* RefCompAudioProcessor::createEditor()
{
    return new RefCompAudioProcessorEditor(*this);
}

void RefCompAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = parameters_.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void RefCompAudioProcessor::setStateInformation(const void* data, const int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters_.state.getType()))
            parameters_.replaceState(juce::ValueTree::fromXml(*xml));
}

} // namespace ehvota::refcomp

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ehvota::refcomp::RefCompAudioProcessor();
}
