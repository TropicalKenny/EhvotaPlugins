#pragma once

#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace ehvota::reftap {

class RefTapAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit RefTapAudioProcessorEditor(RefTapAudioProcessor&);
    ~RefTapAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    RefTapAudioProcessor& processor_;
    juce::Label peakLabel_;
    juce::Label rmsLabel_;
    juce::Label correlationLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefTapAudioProcessorEditor)
};

} // namespace ehvota::reftap
