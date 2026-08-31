#pragma once

#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace ehvota::refcolor {

class RefColorAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit RefColorAudioProcessorEditor(RefColorAudioProcessor&);
    ~RefColorAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    RefColorAudioProcessor& processor_;
    juce::ComboBox mode_;
    juce::Slider amount_;
    juce::Slider drive_;
    juce::Slider asymmetry_;
    juce::Slider output_;
    juce::Label amountLabel_;
    juce::Label driveLabel_;
    juce::Label asymmetryLabel_;
    juce::Label outputLabel_;
    juce::Label qualityLabel_;
    std::unique_ptr<ComboAttachment> modeAttachment_;
    std::unique_ptr<SliderAttachment> amountAttachment_;
    std::unique_ptr<SliderAttachment> driveAttachment_;
    std::unique_ptr<SliderAttachment> asymmetryAttachment_;
    std::unique_ptr<SliderAttachment> outputAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefColorAudioProcessorEditor)
};

} // namespace ehvota::refcolor
