#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace ehvota::refeq {
namespace {

float value(juce::AudioProcessorValueTreeState& state, const char* id)
{
    return state.getRawParameterValue(id)->load();
}

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
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("hpFreq", 1), "High-pass", juce::NormalisableRange<float>(20.0f, 800.0f, 0.01f, 0.35f), 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lpFreq", 1), "Low-pass", juce::NormalisableRange<float>(2000.0f, 22000.0f, 0.01f, 0.35f), 22000.0f));

    const struct BandDefault { const char* prefix; float frequency; } defaults[] {
        { "b1", 120.0f }, { "b2", 450.0f }, { "b3", 2200.0f }, { "b4", 8500.0f }
    };
    for (const auto& band : defaults) {
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(juce::String(band.prefix) + "Freq", 1), juce::String(band.prefix) + " Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 0.01f, 0.25f), band.frequency));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(juce::String(band.prefix) + "Gain", 1), juce::String(band.prefix) + " Gain", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(juce::String(band.prefix) + "Q", 1), juce::String(band.prefix) + " Q", juce::NormalisableRange<float>(0.15f, 18.0f, 0.001f, 0.35f), 0.8f));
    }

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("output", 1), "Output", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.01f), 0.0f));
    return layout;
}

void RefEQAudioProcessor::prepareToPlay(const double sampleRate, const int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    const juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(juce::jmax(1, getTotalNumOutputChannels())) };
    highPass_.prepare(spec);
    band1_.prepare(spec);
    band2_.prepare(spec);
    band3_.prepare(spec);
    band4_.prepare(spec);
    lowPass_.prepare(spec);
    outputGain_.prepare(spec);
    outputGain_.setRampDurationSeconds(0.02);
    updateFilters();
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

void RefEQAudioProcessor::updateFilters()
{
    const auto nyquistSafe = static_cast<float>(sampleRate_ * 0.49);
    const auto hp = juce::jlimit(10.0f, nyquistSafe, value(parameters_, "hpFreq"));
    const auto lp = juce::jlimit(20.0f, nyquistSafe, value(parameters_, "lpFreq"));
    *highPass_.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate_, hp);
    *lowPass_.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, lp);

    Filter* filters[] { &band1_, &band2_, &band3_, &band4_ };
    const char* prefixes[] { "b1", "b2", "b3", "b4" };
    for (int i = 0; i < 4; ++i) {
        const auto prefix = juce::String(prefixes[i]);
        const auto frequency = juce::jlimit(10.0f, nyquistSafe, value(parameters_, (prefix + "Freq").toRawUTF8()));
        const auto gain = juce::Decibels::decibelsToGain(value(parameters_, (prefix + "Gain").toRawUTF8()));
        const auto q = value(parameters_, (prefix + "Q").toRawUTF8());
        *filters[i]->state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate_, frequency, q, gain);
    }

    outputGain_.setGainDecibels(value(parameters_, "output"));
}

void RefEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateFilters();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    highPass_.process(context);
    band1_.process(context);
    band2_.process(context);
    band3_.process(context);
    band4_.process(context);
    lowPass_.process(context);
    outputGain_.process(context);
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
