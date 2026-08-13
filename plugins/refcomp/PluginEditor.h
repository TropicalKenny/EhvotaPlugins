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
    juce::ComboBox mode_;
    juce::ToggleButton attackSync_ { "SYNC ATTACK" };
    juce::ComboBox attackNote_;
    juce::ComboBox attackModifier_;
    juce::ToggleButton releaseSync_ { "SYNC RELEASE" };
    juce::ComboBox releaseNote_;
    juce::ComboBox releaseModifier_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attackSyncAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attackNoteAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attackModifierAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> releaseSyncAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> releaseNoteAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> releaseModifierAttachment_;
    juce::Label gainReductionLabel_;
    juce::Label timingLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefCompAudioProcessorEditor)
};

} // namespace ehvota::refcomp
