#pragma once

#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace ehvota::reftime {

class RefTimeAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit RefTimeAudioProcessorEditor(RefTimeAudioProcessor&);
    ~RefTimeAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void copyText(const juce::String& text);

    RefTimeAudioProcessor& processor_;
    juce::ComboBox divisionSelector_;
    juce::ComboBox modifierSelector_;
    juce::ComboBox outputModeSelector_;
    juce::Label exactLabel_;
    juce::Label hostLabel_;
    juce::Label detailLabel_;
    juce::TextButton copyExactButton_ { "COPY EXACT MS" };
    juce::TextButton copyNumberButton_ { "COPY NUMBER" };
    juce::TextButton copySamplesButton_ { "COPY SAMPLES" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divisionAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modifierAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> outputModeAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefTimeAudioProcessorEditor)
};

} // namespace ehvota::reftime
