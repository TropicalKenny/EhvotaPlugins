#pragma once

#include "shared/timing/RefTimeMath.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <vector>

namespace ehvota::reftime {

class RefTimeAudioProcessor final : public juce::AudioProcessor {
public:
    RefTimeAudioProcessor();
    ~RefTimeAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override;

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState& parameters() noexcept { return parameters_; }
    double hostBpm() const noexcept { return hostBpm_.load(std::memory_order_relaxed); }
    double exactMilliseconds() const noexcept { return exactMilliseconds_.load(std::memory_order_relaxed); }
    std::int64_t exactSamples() const noexcept { return exactSamples_.load(std::memory_order_relaxed); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    float readDelaySample(int channel, double delaySamples) const noexcept;
    void updateTimingTarget();

    juce::AudioProcessorValueTreeState parameters_;
    std::vector<std::vector<float>> delayBuffer_;
    int writePosition_ {};
    int delayBufferSize_ {};
    double sampleRate_ { 44100.0 };
    double activeDelaySamples_ {};
    double previousDelaySamples_ {};
    int crossfadeSamplesRemaining_ {};
    int crossfadeSamplesTotal_ {};

    std::atomic<double> hostBpm_ { 120.0 };
    std::atomic<double> exactMilliseconds_ { 0.0 };
    std::atomic<std::int64_t> exactSamples_ { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefTimeAudioProcessor)
};

} // namespace ehvota::reftime
