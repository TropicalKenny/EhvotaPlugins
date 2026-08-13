#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "plugins/common/SharedProposalApply.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ehvota::refeq {
namespace {

float value(juce::AudioProcessorValueTreeState& state, const char* id)
{
    return state.getRawParameterValue(id)->load(std::memory_order_relaxed);
}

constexpr std::array<const char*, 6> frequencyIds { "b1Freq", "b2Freq", "b3Freq", "b4Freq", "b5Freq", "b6Freq" };
constexpr std::array<const char*, 6> gainIds { "b1Gain", "b2Gain", "b3Gain", "b4Gain", "b5Gain", "b6Gain" };
constexpr std::array<const char*, 6> qIds { "b1Q", "b2Q", "b3Q", "b4Q", "b5Q", "b6Q" };

constexpr std::array<ehvota::plugins::SharedParameterBinding, 24> linkBindings {{
    { ehvota::communication::SharedParameterKey::eqMode, "mode" },
    { ehvota::communication::SharedParameterKey::eqHighPassEnabled, "hpEnabled" },
    { ehvota::communication::SharedParameterKey::eqHighPassFrequency, "hpFreq" },
    { ehvota::communication::SharedParameterKey::eqLowPassEnabled, "lpEnabled" },
    { ehvota::communication::SharedParameterKey::eqLowPassFrequency, "lpFreq" },
    { ehvota::communication::SharedParameterKey::eqBand1Frequency, "b1Freq" },
    { ehvota::communication::SharedParameterKey::eqBand1Gain, "b1Gain" },
    { ehvota::communication::SharedParameterKey::eqBand1Q, "b1Q" },
    { ehvota::communication::SharedParameterKey::eqBand2Frequency, "b2Freq" },
    { ehvota::communication::SharedParameterKey::eqBand2Gain, "b2Gain" },
    { ehvota::communication::SharedParameterKey::eqBand2Q, "b2Q" },
    { ehvota::communication::SharedParameterKey::eqBand3Frequency, "b3Freq" },
    { ehvota::communication::SharedParameterKey::eqBand3Gain, "b3Gain" },
    { ehvota::communication::SharedParameterKey::eqBand3Q, "b3Q" },
    { ehvota::communication::SharedParameterKey::eqBand4Frequency, "b4Freq" },
    { ehvota::communication::SharedParameterKey::eqBand4Gain, "b4Gain" },
    { ehvota::communication::SharedParameterKey::eqBand4Q, "b4Q" },
    { ehvota::communication::SharedParameterKey::eqBand5Frequency, "b5Freq" },
    { ehvota::communication::SharedParameterKey::eqBand5Gain, "b5Gain" },
    { ehvota::communication::SharedParameterKey::eqBand5Q, "b5Q" },
    { ehvota::communication::SharedParameterKey::eqBand6Frequency, "b6Freq" },
    { ehvota::communication::SharedParameterKey::eqBand6Gain, "b6Gain" },
    { ehvota::communication::SharedParameterKey::eqBand6Q, "b6Q" },
    { ehvota::communication::SharedParameterKey::eqOutput, "output" }
}};

constexpr float midSideScale = 0.7071067811865476f;

} // namespace

RefEQAudioProcessor::RefEQAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    startTimerHz(10);
}

RefEQAudioProcessor::~RefEQAudioProcessor() { stopTimer(); }

juce::AudioProcessorValueTreeState::ParameterLayout RefEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mode", 1), "Processing domain",
        juce::StringArray { "Stereo", "Mid", "Side" }, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("hpEnabled",1),"High-pass enabled",false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("hpFreq",1),"High-pass",juce::NormalisableRange<float>(20,800,0.01f,0.35f),30));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("lpEnabled",1),"Low-pass enabled",false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lpFreq",1),"Low-pass",juce::NormalisableRange<float>(2000,22000,0.01f,0.35f),20000));
    constexpr std::array<float,6> defaults { 70, 220, 700, 2200, 6500, 14000 };
    for (std::size_t i = 0; i < defaults.size(); ++i) {
        const auto prefix = "b" + juce::String(static_cast<int>(i + 1));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(prefix+"Freq",1),prefix+" Frequency",juce::NormalisableRange<float>(20,20000,0.01f,0.25f),defaults[i]));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(prefix+"Gain",1),prefix+" Gain",juce::NormalisableRange<float>(-18,18,0.01f),0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(prefix+"Q",1),prefix+" Q",juce::NormalisableRange<float>(0.15f,18,0.001f,0.35f),0.8f));
    }
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("output",1),"Output",juce::NormalisableRange<float>(-18,18,0.01f),0));
    return layout;
}

void RefEQAudioProcessor::resetFilterState() noexcept
{
    highPass_.reset();
    lowPass_.reset();
    for (auto& band : bands_)
        band.filter.reset();
}

void RefEQAudioProcessor::prepareToPlay(const double sampleRate, const int)
{
    sampleRate_ = sampleRate;
    sampleCounter_.store(0, std::memory_order_relaxed);
    activeMode_ = static_cast<int>(value(parameters_, "mode"));
    resetFilterState();
    highPassFrequency_.snap(value(parameters_,"hpFreq"));
    lowPassFrequency_.snap(value(parameters_,"lpFreq"));
    outputDb_.snap(value(parameters_,"output"));
    for (std::size_t i = 0; i < bands_.size(); ++i) {
        bands_[i].frequency.snap(value(parameters_,frequencyIds[i]));
        bands_[i].gainDb.snap(value(parameters_,gainIds[i]));
        bands_[i].q.snap(value(parameters_,qIds[i]));
    }
    updateFilters(0);
}

void RefEQAudioProcessor::releaseResources() {}

bool RefEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    return layout.getMainInputChannelSet() == layout.getMainOutputChannelSet()
        && (layout.getMainInputChannelSet() == juce::AudioChannelSet::mono()
            || layout.getMainInputChannelSet() == juce::AudioChannelSet::stereo());
}

void RefEQAudioProcessor::updateFilters(const int blockSamples) noexcept
{
    const auto seconds = sampleRate_ > 0 ? static_cast<double>(std::max(blockSamples, 1)) / sampleRate_ : 0.0;
    const auto amount = blockSamples <= 0 ? 1.0 : 1.0 - std::exp(-seconds / 0.030);
    const auto hp = highPassFrequency_.follow(value(parameters_,"hpFreq"),amount);
    const auto lp = lowPassFrequency_.follow(value(parameters_,"lpFreq"),amount);
    highPass_.setCoefficients(ehvota::audio::makeHighPass(sampleRate_,hp,0.7071067811865476));
    lowPass_.setCoefficients(ehvota::audio::makeLowPass(sampleRate_,lp,0.7071067811865476));
    for (std::size_t i = 0; i < bands_.size(); ++i) {
        auto& band = bands_[i];
        const auto frequency = band.frequency.follow(value(parameters_,frequencyIds[i]),amount);
        const auto gain = band.gainDb.follow(value(parameters_,gainIds[i]),amount);
        const auto q = band.q.follow(value(parameters_,qIds[i]),amount);
        band.activeGainDb = gain;
        band.filter.setCoefficients(ehvota::audio::makePeaking(sampleRate_,frequency,q,gain));
    }
    activeOutputGain_ = std::pow(10.0, outputDb_.follow(value(parameters_,"output"),amount) / 20.0);
}

bool RefEQAudioProcessor::isNeutral() const noexcept
{
    if (value(const_cast<juce::AudioProcessorValueTreeState&>(parameters_),"hpEnabled") >= 0.5f
        || value(const_cast<juce::AudioProcessorValueTreeState&>(parameters_),"lpEnabled") >= 0.5f
        || std::abs(activeOutputGain_ - 1.0) > 1.0e-7)
        return false;
    return std::all_of(bands_.begin(), bands_.end(), [](const auto& band) { return std::abs(band.activeGainDb) < 1.0e-6; });
}

float RefEQAudioProcessor::processDomainSample(const std::size_t domain, float sample, const bool hp, const bool lp) noexcept
{
    if (hp) sample = highPass_.process(domain, sample);
    for (auto& band : bands_)
        if (std::abs(band.activeGainDb) >= 1.0e-6)
            sample = band.filter.process(domain, sample);
    if (lp) sample = lowPass_.process(domain, sample);
    return sample;
}

void RefEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    sampleCounter_.fetch_add(static_cast<std::uint64_t>(buffer.getNumSamples()), std::memory_order_relaxed);
    updateFilters(buffer.getNumSamples());
    if (isNeutral())
        return; // Exact untouched-buffer path at default settings, regardless of selected domain.

    const bool hp = value(parameters_,"hpEnabled") >= 0.5f;
    const bool lp = value(parameters_,"lpEnabled") >= 0.5f;
    const int channels = std::min(buffer.getNumChannels(), 2);
    const int requestedMode = juce::jlimit(0, 2, static_cast<int>(value(parameters_,"mode")));
    const int effectiveMode = channels >= 2 ? requestedMode : 0;
    if (effectiveMode != activeMode_) {
        resetFilterState();
        activeMode_ = effectiveMode;
    }

    if (effectiveMode == 0) {
        for (int channel = 0; channel < channels; ++channel) {
            auto* samples = buffer.getWritePointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                samples[i] = static_cast<float>(static_cast<double>(processDomainSample(static_cast<std::size_t>(channel), samples[i], hp, lp)) * activeOutputGain_);
        }
        return;
    }

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const std::size_t selectedDomain = effectiveMode == 1 ? 0u : 1u;
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        const float inputLeft = left[i];
        const float inputRight = right[i];
        float mid = (inputLeft + inputRight) * midSideScale;
        float side = (inputLeft - inputRight) * midSideScale;
        if (effectiveMode == 1)
            mid = processDomainSample(selectedDomain, mid, hp, lp);
        else
            side = processDomainSample(selectedDomain, side, hp, lp);
        left[i] = static_cast<float>(static_cast<double>((mid + side) * midSideScale) * activeOutputGain_);
        right[i] = static_cast<float>(static_cast<double>((mid - side) * midSideScale) * activeOutputGain_);
    }
}

void RefEQAudioProcessor::timerCallback()
{
    refLink_.publish({-100,-100,1,0,sampleCounter_.load(std::memory_order_relaxed)});
    if (auto proposal = refLink_.consumePendingProposal()) {
        const auto result = ehvota::plugins::applySharedProposal(parameters_,ehvota::communication::SharedPluginKind::refEQ,*proposal,linkBindings);
        const auto state = result == ehvota::plugins::SharedProposalApplyResult::applied
            ? ehvota::communication::SharedProposalState::applied
            : ehvota::communication::SharedProposalState::rejected;
        refLink_.completeProposal(proposal->sequence,state,static_cast<std::uint32_t>(result));
    }
}

juce::AudioProcessorEditor* RefEQAudioProcessor::createEditor() { return new RefEQAudioProcessorEditor(*this); }
void RefEQAudioProcessor::getStateInformation(juce::MemoryBlock& destination) { if (auto xml=parameters_.copyState().createXml()) copyXmlToBinary(*xml,destination); }
void RefEQAudioProcessor::setStateInformation(const void* data,int bytes) { if (auto xml=getXmlFromBinary(data,bytes)) if (xml->hasTagName(parameters_.state.getType())) parameters_.replaceState(juce::ValueTree::fromXml(*xml)); }

} // namespace ehvota::refeq

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ehvota::refeq::RefEQAudioProcessor(); }
