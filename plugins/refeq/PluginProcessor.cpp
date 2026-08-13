#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ehvota::refeq {
namespace {

float value(juce::AudioProcessorValueTreeState& state, const char* id)
{
    return state.getRawParameterValue(id)->load(std::memory_order_relaxed);
}

constexpr std::array<const char*, 6> frequencyIds {
    "b1Freq", "b2Freq", "b3Freq", "b4Freq", "b5Freq", "b6Freq"
};
constexpr std::array<const char*, 6> gainIds {
    "b1Gain", "b2Gain", "b3Gain", "b4Gain", "b5Gain", "b6Gain"
};
constexpr std::array<const char*, 6> qIds {
    "b1Q", "b2Q", "b3Q", "b4Q", "b5Q", "b6Q"
};

} // namespace

RefEQAudioProcessor::RefEQAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout RefEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("hpEnabled", 1), "High-pass enabled", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("hpFreq", 1), "High-pass", juce::NormalisableRange<float>(20.0f, 800.0f, 0.01f, 0.35f), 30.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("lpEnabled", 1), "Low-pass enabled", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lpFreq", 1), "Low-pass", juce::NormalisableRange<float>(2000.0f, 22000.0f, 0.01f, 0.35f), 20000.0f));

    constexpr std::array<float, 6> defaults { 70.0f, 220.0f, 700.0f, 2200.0f, 6500.0f, 14000.0f };
    for (std::size_t i = 0; i < defaults.size(); ++i) {
        const auto prefix = "b" + juce::String(static_cast<int>(i + 1));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "Freq", 1), prefix + " Frequency",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 0.01f, 0.25f), defaults[i]));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "Gain", 1), prefix + " Gain",
            juce::NormalisableRange<float>(-18.0f, 18.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "Q", 1), prefix + " Q",
            juce::NormalisableRange<float>(0.15f, 18.0f, 0.001f, 0.35f), 0.8f));
    }

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("output", 1), "Output",
        juce::NormalisableRange<float>(-18.0f, 18.0f, 0.01f), 0.0f));
    return layout;
}

void RefEQAudioProcessor::prepareToPlay(const double sampleRate, const int)
{
    sampleRate_ = sampleRate;
    highPass_.reset();
    lowPass_.reset();
    for (auto& band : bands_)
        band.filter.reset();

    highPassFrequency_.snap(value(parameters_, "hpFreq"));
    lowPassFrequency_.snap(value(parameters_, "lpFreq"));
    outputDb_.snap(value(parameters_, "output"));
    for (std::size_t i = 0; i < bands_.size(); ++i) {
        bands_[i].frequency.snap(value(parameters_, frequencyIds[i]));
        bands_[i].gainDb.snap(value(parameters_, gainIds[i]));
        bands_[i].q.snap(value(parameters_, qIds[i]));
    }
    updateFilters(0);
}

void RefEQAudioProcessor::releaseResources()
{
}

bool RefEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void RefEQAudioProcessor::updateFilters(const int blockSamples) noexcept
{
    const auto seconds = sampleRate_ > 0.0
        ? static_cast<double>(std::max(blockSamples, 1)) / sampleRate_
        : 0.0;
    const auto amount = blockSamples <= 0
        ? 1.0
        : 1.0 - std::exp(-seconds / 0.030);

    const auto hpFrequency = highPassFrequency_.follow(value(parameters_, "hpFreq"), amount);
    const auto lpFrequency = lowPassFrequency_.follow(value(parameters_, "lpFreq"), amount);
    highPass_.setCoefficients(ehvota::audio::makeHighPass(sampleRate_, hpFrequency, 0.7071067811865476));
    lowPass_.setCoefficients(ehvota::audio::makeLowPass(sampleRate_, lpFrequency, 0.7071067811865476));

    for (std::size_t i = 0; i < bands_.size(); ++i) {
        auto& band = bands_[i];
        const auto frequency = band.frequency.follow(value(parameters_, frequencyIds[i]), amount);
        const auto gainDb = band.gainDb.follow(value(parameters_, gainIds[i]), amount);
        const auto q = band.q.follow(value(parameters_, qIds[i]), amount);
        band.activeGainDb = gainDb;
        band.filter.setCoefficients(ehvota::audio::makePeaking(sampleRate_, frequency, q, gainDb));
    }

    const auto outputDb = outputDb_.follow(value(parameters_, "output"), amount);
    activeOutputGain_ = std::pow(10.0, outputDb / 20.0);
}

bool RefEQAudioProcessor::isNeutral() const noexcept
{
    if (value(const_cast<juce::AudioProcessorValueTreeState&>(parameters_), "hpEnabled") >= 0.5f)
        return false;
    if (value(const_cast<juce::AudioProcessorValueTreeState&>(parameters_), "lpEnabled") >= 0.5f)
        return false;
    if (std::abs(activeOutputGain_ - 1.0) > 1.0e-7)
        return false;
    return std::all_of(bands_.begin(), bands_.end(), [](const auto& band) {
        return std::abs(band.activeGainDb) < 1.0e-6;
    });
}

void RefEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateFilters(buffer.getNumSamples());

    if (isNeutral())
        return; // Exact untouched-buffer path at default settings.

    const bool hpEnabled = value(parameters_, "hpEnabled") >= 0.5f;
    const bool lpEnabled = value(parameters_, "lpEnabled") >= 0.5f;
    const auto channels = std::min(buffer.getNumChannels(), 2);

    for (int channel = 0; channel < channels; ++channel) {
        auto* samples = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float x = samples[sample];
            if (hpEnabled)
                x = highPass_.process(static_cast<std::size_t>(channel), x);
            for (auto& band : bands_)
                if (std::abs(band.activeGainDb) >= 1.0e-6)
                    x = band.filter.process(static_cast<std::size_t>(channel), x);
            if (lpEnabled)
                x = lowPass_.process(static_cast<std::size_t>(channel), x);
            samples[sample] = static_cast<float>(static_cast<double>(x) * activeOutputGain_);
        }
    }
}

juce::AudioProcessorEditor* RefEQAudioProcessor::createEditor()
{
    return new RefEQAudioProcessorEditor(*this);
}

void RefEQAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = parameters_.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void RefEQAudioProcessor::setStateInformation(const void* data, const int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters_.state.getType()))
            parameters_.replaceState(juce::ValueTree::fromXml(*xml));
}

} // namespace ehvota::refeq

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ehvota::refeq::RefEQAudioProcessor();
}
