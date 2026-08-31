#pragma once

#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

namespace ehvota::reftransient {

class RefTransientAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit RefTransientAudioProcessorEditor(RefTransientAudioProcessor&);
    ~RefTransientAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    RefTransientAudioProcessor& processor_;
    std::array<juce::Slider, 4> controls_;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 4> attachments_;
    juce::Label activityLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefTransientAudioProcessorEditor)
};

} // namespace ehvota::reftransient
