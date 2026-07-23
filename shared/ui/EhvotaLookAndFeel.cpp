#include "EhvotaLookAndFeel.h"

#include "Theme.h"

namespace ehvota::ui {

EhvotaLookAndFeel::EhvotaLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(colour::canvas));
    setColour(juce::TextButton::buttonColourId, juce::Colour(colour::raised));
    setColour(juce::TextButton::buttonOnColourId, juce::Colour(colour::overlay));
    setColour(juce::TextButton::textColourOffId, juce::Colour(colour::textSecondary));
    setColour(juce::TextButton::textColourOnId, juce::Colour(colour::textPrimary));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(colour::raised));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(colour::borderSubtle));
    setColour(juce::ComboBox::textColourId, juce::Colour(colour::textPrimary));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(colour::textSecondary));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(colour::base));
    setColour(juce::PopupMenu::textColourId, juce::Colour(colour::textPrimary));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(colour::overlay));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(colour::textPrimary));
    setColour(juce::Label::textColourId, juce::Colour(colour::textPrimary));
    setColour(juce::TooltipWindow::backgroundColourId, juce::Colour(colour::overlay));
    setColour(juce::TooltipWindow::textColourId, juce::Colour(colour::textPrimary));
    setColour(juce::TooltipWindow::outlineColourId, juce::Colour(colour::borderStrong));
}

juce::Font EhvotaLookAndFeel::getTextButtonFont(juce::TextButton&, const int buttonHeight)
{
    return font(juce::jmin(13.0f, static_cast<float>(buttonHeight) * 0.42f), true);
}

juce::Font EhvotaLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return font(13.0f, true);
}

void EhvotaLookAndFeel::drawButtonBackground(
    juce::Graphics& graphics,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    const bool isMouseOverButton,
    const bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto fill = backgroundColour;

    if (isButtonDown)
        fill = fill.brighter(0.16f);
    else if (isMouseOverButton)
        fill = fill.brighter(0.08f);

    graphics.setColour(fill);
    graphics.fillRoundedRectangle(bounds, 5.0f);

    const auto border = button.hasKeyboardFocus(true)
        ? juce::Colour(colour::focus)
        : juce::Colour(colour::borderSubtle);
    graphics.setColour(border);
    graphics.drawRoundedRectangle(bounds, 5.0f, button.hasKeyboardFocus(true) ? 2.0f : 1.0f);
}

} // namespace ehvota::ui
