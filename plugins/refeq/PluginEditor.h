#pragma once

#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

namespace ehvota::refeq {

class RefEQAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit RefEQAudioProcessorEditor(RefEQAudioProcessor&);
    ~RefEQAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct BandControls {
        juce::Slider frequency;
        juce::Slider gain;
        juce::Slider q;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> qAttachment;
    };

    void configureKnob(juce::Slider&, const juce::String& suffix);

    RefEQAudioProcessor& processor_;
    std::array<BandControls, 4> bands_;
    juce::Slider highPass_;
    juce::Slider lowPass_;
    juce::Slider output_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highPassAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowPassAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefEQAudioProcessorEditor)
};

} // namespace ehvota::refeq
