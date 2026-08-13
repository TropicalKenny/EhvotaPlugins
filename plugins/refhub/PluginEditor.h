#pragma once

#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace ehvota::refhub {

class RefHubAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit RefHubAudioProcessorEditor(RefHubAudioProcessor&);
    ~RefHubAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    RefHubAudioProcessor& processor_;
    juce::ComboBox referenceSelector_;
    juce::ComboBox sectionSelector_;
    juce::ComboBox levelMatchSelector_;
    juce::TextButton mixButton_ { "MIX" };
    juce::TextButton referenceButton_ { "REF" };
    juce::TextButton blindButton_ { "BLIND" };
    juce::TextButton captureButton_ { "CAPTURE" };
    juce::TextButton proposalButton_ { "CREATE PROPOSAL" };
    juce::Label liveMeterLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefHubAudioProcessorEditor)
};

} // namespace ehvota::refhub
