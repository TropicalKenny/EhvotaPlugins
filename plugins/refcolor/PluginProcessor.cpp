#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <cmath>

namespace ehvota::refcolor {
namespace {

float parameterValue(juce::AudioProcessorValueTreeState& state, const char* id)
{
    return state.getRawParameterValue(id)->load(std::memory_order_relaxed);
}

struct ModeProfile {
    double driveOffsetDb {};
    double asymmetryOffset {};
    double characterScale { 1.0 };
};

ModeProfile profileForMode(const int mode) noexcept
{
    switch (mode) {
    case 1: return { 2.0, 0.02, 1.15 }; // Tape: more symmetric peak rounding.
    case 2: return { 1.0, 0.24, 0.90 }; // Transformer: stronger even-harmonic bias.
    default: return { 0.0, 0.10, 1.00 }; // Console: restrained default colour.
    }
}

} // namespace

RefColorAudioProcessor::RefColorAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, "PARAMETERS", createParameterLayout()),
      oversampling_(2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout RefColorAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mode", 1), "Character",
        juce::StringArray { "Console", "Tape", "Transformer" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("amount", 1), "Amount",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 18.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("drive", 1), "Drive",
        juce::NormalisableRange<float>(0.0f, 18.0f, 0.01f), 3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("asymmetry", 1), "Asymmetry",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("output", 1), "Output",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f));
    return layout;
}

void RefColorAudioProcessor::prepareToPlay(const double sampleRate, const int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    oversampling_.reset();
    oversampling_.initProcessing(static_cast<std::size_t>(std::max(samplesPerBlock, 1)));
    setLatencySamples(static_cast<int>(std::lround(oversampling_.getLatencyInSamples())));

    const auto oversampledRate = sampleRate * 4.0;
    amount_.reset(oversampledRate, 0.050);
    driveDb_.reset(oversampledRate, 0.050);
    outputDb_.reset(oversampledRate, 0.050);
    amount_.setCurrentAndTargetValue(parameterValue(parameters_, "amount") * 0.01f);
    driveDb_.setCurrentAndTargetValue(parameterValue(parameters_, "drive"));
    outputDb_.setCurrentAndTargetValue(parameterValue(parameters_, "output"));
    dcBlocker_.prepare(oversampledRate, 7.0);
    dcBlocker_.reset();
}

void RefColorAudioProcessor::releaseResources()
{
    oversampling_.reset();
}

bool RefColorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void RefColorAudioProcessor::updateSmoothingTargets() noexcept
{
    amount_.setTargetValue(parameterValue(parameters_, "amount") * 0.01f);
    driveDb_.setTargetValue(parameterValue(parameters_, "drive"));
    outputDb_.setTargetValue(parameterValue(parameters_, "output"));
}

void RefColorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (buffer.getNumSamples() <= 0 || buffer.getNumChannels() <= 0)
        return;

    updateSmoothingTargets();
    const auto mode = static_cast<int>(parameterValue(parameters_, "mode"));
    const auto profile = profileForMode(mode);
    const auto userAsymmetry = static_cast<double>(parameterValue(parameters_, "asymmetry"));

    juce::dsp::AudioBlock<float> block(buffer);
    auto oversampled = oversampling_.processSamplesUp(block);

    const auto channels = std::min<std::size_t>(oversampled.getNumChannels(), 2);
    const auto samples = oversampled.getNumSamples();
    for (std::size_t sample = 0; sample < samples; ++sample) {
        ehvota::audio::AnalogSaturationSettings settings;
        settings.character = std::clamp(
            static_cast<double>(amount_.getNextValue()) * profile.characterScale, 0.0, 1.0);
        settings.driveDb = std::clamp(
            static_cast<double>(driveDb_.getNextValue()) + profile.driveOffsetDb, 0.0, 24.0);
        settings.asymmetry = std::clamp(userAsymmetry + profile.asymmetryOffset, -1.0, 1.0);
        settings.outputDb = static_cast<double>(outputDb_.getNextValue());

        for (std::size_t channel = 0; channel < channels; ++channel) {
            auto* data = oversampled.getChannelPointer(channel);
            const auto shaped = ehvota::audio::analogTransfer(static_cast<double>(data[sample]), settings);
            data[sample] = dcBlocker_.process(channel, static_cast<float>(shaped));
        }
    }

    oversampling_.processSamplesDown(block);
}

juce::AudioProcessorEditor* RefColorAudioProcessor::createEditor()
{
    return new RefColorAudioProcessorEditor(*this);
}

void RefColorAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = parameters_.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void RefColorAudioProcessor::setStateInformation(const void* data, const int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters_.state.getType()))
            parameters_.replaceState(juce::ValueTree::fromXml(*xml));
}

} // namespace ehvota::refcolor

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ehvota::refcolor::RefColorAudioProcessor();
}
