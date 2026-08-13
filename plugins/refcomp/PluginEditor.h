#pragma once

#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

namespace ehvota::refcomp {

class RefCompAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit RefCompAudioProcessorEditor(RefCompAudioProcessor&);
    ~RefCompAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void configureKnob(juce::Slider&, const juce::String& suffix);

    RefCompAudioProcessor& processor_;
    std::array<juce::Slider, 7> controls_;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 7> attachments_;
    juce::Label gainReductionLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefCompAudioProcessorEditor)
};

} // namespace ehvota::refcomp
