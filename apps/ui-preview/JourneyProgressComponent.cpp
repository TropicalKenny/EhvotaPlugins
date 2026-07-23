#include "JourneyProgressComponent.h"

#include "shared/ui/Theme.h"

namespace ehvota::previewapp {

JourneyProgressComponent::JourneyProgressComponent()
{
    setInterceptsMouseClicks(false, false);
}

void JourneyProgressComponent::setStep(
    const int journeyNumber,
    juce::String journeyTitle,
    const int currentStep,
    const int totalSteps,
    juce::String stepTitle,
    juce::String instruction)
{
    journeyNumber_ = journeyNumber;
    journeyTitle_ = std::move(journeyTitle);
    currentStep_ = currentStep;
    totalSteps_ = totalSteps;
    stepTitle_ = std::move(stepTitle);
    instruction_ = std::move(instruction);

    setTitle("Journey " + juce::String(journeyNumber_) + ": " + journeyTitle_);
    setDescription(
        "Step " + juce::String(currentStep_) + " of " + juce::String(totalSteps_)
        + ": " + stepTitle_ + ". " + instruction_);
    repaint();
}

void JourneyProgressComponent::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    graphics.setColour(juce::Colour(ui::colour::base));
    graphics.fillRoundedRectangle(bounds, 8.0f);
    graphics.setColour(juce::Colour(ui::colour::borderSubtle));
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    auto content = getLocalBounds().reduced(14, 9);
    content.removeFromRight(164);

    auto labelArea = content.removeFromLeft(136);
    graphics.setColour(juce::Colour(ui::colour::proposed));
    graphics.setFont(ui::font(11.0f, true));
    graphics.drawText(
        "JOURNEY " + juce::String(journeyNumber_),
        labelArea.removeFromTop(18),
        juce::Justification::centredLeft);
    graphics.setColour(juce::Colour(ui::colour::textMuted));
    graphics.setFont(ui::numericFont(11.0f, true));
    graphics.drawText(
        juce::String(currentStep_) + " / " + juce::String(totalSteps_),
        labelArea.removeFromTop(18),
        juce::Justification::centredLeft);
    graphics.setColour(juce::Colour(ui::colour::textSecondary));
    graphics.setFont(ui::font(10.0f));
    graphics.drawFittedText(
        journeyTitle_,
        labelArea,
        juce::Justification::topLeft,
        1);

    content.removeFromLeft(8);
    graphics.setColour(juce::Colour(ui::colour::textPrimary));
    graphics.setFont(ui::font(13.0f, true));
    graphics.drawFittedText(
        stepTitle_,
        content.removeFromTop(21),
        juce::Justification::centredLeft,
        1);
    graphics.setColour(juce::Colour(ui::colour::textSecondary));
    graphics.setFont(ui::font(11.0f));
    graphics.drawFittedText(
        instruction_,
        content,
        juce::Justification::topLeft,
        2);

    auto track = getLocalBounds().toFloat().reduced(1.0f).removeFromBottom(3.0f);
    graphics.setColour(juce::Colour(ui::colour::overlay));
    graphics.fillRoundedRectangle(track, 1.5f);
    const auto progress = totalSteps_ > 0
        ? static_cast<float>(currentStep_) / static_cast<float>(totalSteps_)
        : 0.0f;
    graphics.setColour(juce::Colour(ui::colour::proposed));
    graphics.fillRoundedRectangle(track.withWidth(track.getWidth() * progress), 1.5f);
}

std::unique_ptr<juce::AccessibilityHandler>
JourneyProgressComponent::createAccessibilityHandler()
{
    return std::make_unique<juce::AccessibilityHandler>(
        *this,
        juce::AccessibilityRole::group);
}

} // namespace ehvota::previewapp
