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
    void configureToggle(juce::ToggleButton&);

    RefEQAudioProcessor& processor_;
    std::array<BandControls, 6> bands_;
    juce::ComboBox mode_;
    juce::ToggleButton highPassEnabled_ { "HP" };
    juce::ToggleButton lowPassEnabled_ { "LP" };
    juce::Slider highPass_;
    juce::Slider lowPass_;
    juce::Slider output_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> highPassEnabledAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lowPassEnabledAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highPassAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowPassAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefEQAudioProcessorEditor)
};

} // namespace ehvota::refeq
