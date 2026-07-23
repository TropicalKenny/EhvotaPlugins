#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace ehvota::ui {

class EhvotaLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    EhvotaLookAndFeel();

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;

    void drawButtonBackground(
        juce::Graphics&,
        juce::Button&,
        const juce::Colour& backgroundColour,
        bool isMouseOverButton,
        bool isButtonDown) override;
};

} // namespace ehvota::ui
