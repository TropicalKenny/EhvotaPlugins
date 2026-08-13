#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "shared/timing/RefTimeMath.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ehvota::refcomp {
namespace {

float value(juce::AudioProcessorValueTreeState& state, const char* id)
{
    return state.getRawParameterValue(id)->load(std::memory_order_relaxed);
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

ehvota::timing::NoteModifier modifierForChoice(const int choice) noexcept
{
    switch (choice) {
    case 1: return ehvota::timing::NoteModifier::dotted;
    case 2: return ehvota::timing::NoteModifier::triplet;
    default: return ehvota::timing::NoteModifier::straight;
    }
}

float syncedMilliseconds(
    const double bpm,
    const double sampleRate,
    const int noteChoice,
    const int modifierChoice,
    const bool attack) noexcept
{
    constexpr std::array<int, 5> attackDenominators { 256, 128, 64, 32, 16 };
    constexpr std::array<int, 5> releaseDenominators { 16, 8, 4, 2, 1 };
    const auto index = static_cast<std::size_t>(juce::jlimit(0, 4, noteChoice));
    const auto denominator = attack ? attackDenominators[index] : releaseDenominators[index];
    const auto timing = ehvota::timing::calculateNoteTiming(
        bpm,
        denominator,
        modifierForChoice(modifierChoice),
        sampleRate);
    return static_cast<float>(timing.exactMilliseconds);
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
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mode", 1), "Processing mode",
        juce::StringArray { "Stereo Linked", "Dual Mono", "Mid / Side" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("threshold", 1), "Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -18.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("ratio", 1), "Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.01f, 0.45f), 4.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("attack", 1), "Attack", juce::NormalisableRange<float>(0.05f, 200.0f, 0.01f, 0.35f), 20.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("attackSync", 1), "Attack sync", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("attackNote", 1), "Attack note",
        juce::StringArray { "1/256", "1/128", "1/64", "1/32", "1/16" }, 2));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("attackModifier", 1), "Attack modifier",
        juce::StringArray { "Straight", "Dotted", "Triplet" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("release", 1), "Release", juce::NormalisableRange<float>(5.0f, 2000.0f, 0.01f, 0.35f), 180.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("releaseSync", 1), "Release sync", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("releaseNote", 1), "Release note",
        juce::StringArray { "1/16", "1/8", "1/4", "1/2", "1/1" }, 2));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("releaseModifier", 1), "Release modifier",
        juce::StringArray { "Straight", "Dotted", "Triplet" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("knee", 1), "Knee", juce::NormalisableRange<float>(0.0f, 24.0f, 0.01f), 6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("makeup", 1), "Makeup", juce::NormalisableRange<float>(-12.0f, 18.0f, 0.01f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("mix", 1), "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 100.0f));
    return layout;
}

void RefCompAudioProcessor::prepareToPlay(const double sampleRate, const int)
{
    sampleRate_ = sampleRate;
    envelopeDb_.fill(-120.0f);
    gainReductionDb_.store(0.0f, std::memory_order_relaxed);
    activeAttackMs_.store(value(parameters_, "attack"), std::memory_order_relaxed);
    activeReleaseMs_.store(value(parameters_, "release"), std::memory_order_relaxed);
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
    if (buffer.getNumSamples() <= 0 || buffer.getNumChannels() <= 0)
        return;

    const auto threshold = value(parameters_, "threshold");
    const auto ratio = value(parameters_, "ratio");
    auto attackMs = value(parameters_, "attack");
    auto releaseMs = value(parameters_, "release");
    const auto kneeDb = value(parameters_, "knee");
    const auto makeupGain = juce::Decibels::decibelsToGain(value(parameters_, "makeup"));
    const auto wet = value(parameters_, "mix") * 0.01f;
    const auto dryMix = 1.0f - wet;
    const auto mode = static_cast<int>(value(parameters_, "mode"));

    double bpm = 120.0;
    if (auto* playHead = getPlayHead()) {
        if (auto position = playHead->getPosition()) {
            if (auto hostBpm = position->getBpm())
                bpm = *hostBpm;
        }
    }

    if (value(parameters_, "attackSync") >= 0.5f) {
        attackMs = syncedMilliseconds(
            bpm,
            sampleRate_,
            static_cast<int>(value(parameters_, "attackNote")),
            static_cast<int>(value(parameters_, "attackModifier")),
            true);
    }
    if (value(parameters_, "releaseSync") >= 0.5f) {
        releaseMs = syncedMilliseconds(
            bpm,
            sampleRate_,
            static_cast<int>(value(parameters_, "releaseNote")),
            static_cast<int>(value(parameters_, "releaseModifier")),
            false);
    }

    attackMs = juce::jlimit(0.05f, 4000.0f, attackMs);
    releaseMs = juce::jlimit(1.0f, 8000.0f, releaseMs);
    activeAttackMs_.store(attackMs, std::memory_order_relaxed);
    activeReleaseMs_.store(releaseMs, std::memory_order_relaxed);

    const auto attackCoeff = std::exp(-1.0 / (0.001 * static_cast<double>(attackMs) * sampleRate_));
    const auto releaseCoeff = std::exp(-1.0 / (0.001 * static_cast<double>(releaseMs) * sampleRate_));

    auto reductionForDetector = [&](const std::size_t envelopeIndex, const float detector) {
        const auto detectorDb = juce::Decibels::gainToDecibels(detector, -120.0f);
        auto& envelope = envelopeDb_[envelopeIndex];
        const auto coefficient = detectorDb > envelope ? attackCoeff : releaseCoeff;
        envelope = static_cast<float>(coefficient * envelope + (1.0 - coefficient) * detectorDb);
        return computeGainReductionDb(envelope, threshold, ratio, kneeDb);
    };

    float minimumGainReduction = 0.0f;
    const auto channels = buffer.getNumChannels();
    constexpr float midSideScale = 0.7071067811865476f;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        if (mode == 2 && channels >= 2) {
            const auto dryLeft = buffer.getSample(0, sample);
            const auto dryRight = buffer.getSample(1, sample);
            const auto mid = (dryLeft + dryRight) * midSideScale;
            const auto side = (dryLeft - dryRight) * midSideScale;

            const auto midReduction = reductionForDetector(0, std::abs(mid));
            const auto sideReduction = reductionForDetector(1, std::abs(side));
            minimumGainReduction = juce::jmin(minimumGainReduction, juce::jmin(midReduction, sideReduction));

            const auto midGain = juce::Decibels::decibelsToGain(midReduction) * makeupGain;
            const auto sideGain = juce::Decibels::decibelsToGain(sideReduction) * makeupGain;
            const auto wetMid = mid * midGain;
            const auto wetSide = side * sideGain;
            const auto wetLeft = (wetMid + wetSide) * midSideScale;
            const auto wetRight = (wetMid - wetSide) * midSideScale;
            buffer.setSample(0, sample, dryLeft * dryMix + wetLeft * wet);
            buffer.setSample(1, sample, dryRight * dryMix + wetRight * wet);
            continue;
        }

        if (mode == 1 && channels >= 2) {
            for (int channel = 0; channel < std::min(channels, 2); ++channel) {
                const auto dry = buffer.getSample(channel, sample);
                const auto reductionDb = reductionForDetector(static_cast<std::size_t>(channel), std::abs(dry));
                minimumGainReduction = juce::jmin(minimumGainReduction, reductionDb);
                const auto compressedGain = juce::Decibels::decibelsToGain(reductionDb) * makeupGain;
                buffer.setSample(channel, sample, dry * dryMix + dry * compressedGain * wet);
            }
            continue;
        }

        float detector = 0.0f;
        for (int channel = 0; channel < channels; ++channel)
            detector = juce::jmax(detector, std::abs(buffer.getSample(channel, sample)));
        const auto reductionDb = reductionForDetector(0, detector);
        envelopeDb_[1] = envelopeDb_[0];
        minimumGainReduction = juce::jmin(minimumGainReduction, reductionDb);
        const auto compressedGain = juce::Decibels::decibelsToGain(reductionDb) * makeupGain;

        for (int channel = 0; channel < channels; ++channel) {
            const auto dry = buffer.getSample(channel, sample);
            buffer.setSample(channel, sample, dry * dryMix + dry * compressedGain * wet);
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
