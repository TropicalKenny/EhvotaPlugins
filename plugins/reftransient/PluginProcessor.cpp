#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace ehvota::reftransient {
namespace {

float value(juce::AudioProcessorValueTreeState& state, const char* id)
{
    return state.getRawParameterValue(id)->load(std::memory_order_relaxed);
}

} // namespace

RefTransientAudioProcessor::RefTransientAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout RefTransientAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("attack", 1), "Attack",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.01f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sustain", 1), "Sustain",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.01f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sensitivity", 1), "Sensitivity",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 50.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("output", 1), "Output",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f));
    return layout;
}

void RefTransientAudioProcessor::prepareToPlay(const double sampleRate, const int)
{
    shaper_.prepare(sampleRate);
    for (auto* smooth : { &attack_, &sustain_, &sensitivity_, &outputDb_ })
        smooth->reset(sampleRate, 0.030);
    attack_.setCurrentAndTargetValue(value(parameters_, "attack"));
    sustain_.setCurrentAndTargetValue(value(parameters_, "sustain"));
    sensitivity_.setCurrentAndTargetValue(value(parameters_, "sensitivity"));
    outputDb_.setCurrentAndTargetValue(value(parameters_, "output"));
    transientActivity_.store(0.0f, std::memory_order_relaxed);
}

void RefTransientAudioProcessor::releaseResources()
{
}

bool RefTransientAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void RefTransientAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (buffer.getNumSamples() <= 0 || buffer.getNumChannels() <= 0)
        return;

    attack_.setTargetValue(value(parameters_, "attack"));
    sustain_.setTargetValue(value(parameters_, "sustain"));
    sensitivity_.setTargetValue(value(parameters_, "sensitivity"));
    outputDb_.setTargetValue(value(parameters_, "output"));

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        ehvota::audio::TransientShaperSettings settings;
        settings.attackPercent = attack_.getNextValue();
        settings.sustainPercent = sustain_.getNextValue();
        settings.sensitivityPercent = sensitivity_.getNextValue();
        settings.outputDb = outputDb_.getNextValue();
        shaper_.setSettings(settings);

        const auto left = buffer.getSample(0, sample);
        const auto right = buffer.getNumChannels() > 1 ? buffer.getSample(1, sample) : left;
        const auto gain = shaper_.gainForFrame(left, right);
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample(channel, sample, buffer.getSample(channel, sample) * gain);
    }

    transientActivity_.store(shaper_.lastTransientAmount(), std::memory_order_relaxed);
}

juce::AudioProcessorEditor* RefTransientAudioProcessor::createEditor()
{
    return new RefTransientAudioProcessorEditor(*this);
}

void RefTransientAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = parameters_.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void RefTransientAudioProcessor::setStateInformation(const void* data, const int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters_.state.getType()))
            parameters_.replaceState(juce::ValueTree::fromXml(*xml));
}

} // namespace ehvota::reftransient

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ehvota::reftransient::RefTransientAudioProcessor();
}
