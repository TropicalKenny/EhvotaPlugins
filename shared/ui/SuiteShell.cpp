#include "SuiteShell.h"

#include "Theme.h"

#include <array>
#include <utility>

namespace ehvota::ui {
namespace {

constexpr int controlGap = 5;

void placeFromLeft(juce::Rectangle<int>& area, juce::Component& component, const int width)
{
    component.setBounds(area.removeFromLeft(width));
    area.removeFromLeft(controlGap);
    component.setVisible(true);
}

} // namespace

SuiteShell::SuiteShell()
{
    setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);

    identityLabel_.setJustificationType(juce::Justification::centredLeft);
    identityLabel_.setFont(font(13.0f, true));
    identityLabel_.setColour(
        juce::Label::textColourId,
        juce::Colour(colour::textPrimary));
    identityLabel_.setTooltip("Ehvota product identity.");

    presetSelector_.setTooltip("Select a local preset. Ctrl+P.");
    presetSelector_.onChange = [this] { emit(ShellAction::selectPreset); };

    previousPresetButton_.setTooltip("Select the previous preset.");
    nextPresetButton_.setTooltip("Select the next preset.");
    savePresetButton_.setTooltip("Save the current preset. Ctrl+S.");
    undoButton_.setTooltip("Undo the latest scoped UI action. Ctrl+Z.");
    redoButton_.setTooltip("Redo the latest scoped UI action. Ctrl+Shift+Z.");
    comparisonButton_.setTooltip("Switch between A and B comparison states.");
    processingButton_.setTooltip("Toggle the product-specific processing or analysis state.");
    helpButton_.setTooltip("Open context-sensitive help. F1.");
    scaleButton_.setTooltip("Cycle the interface scale preview.");
    overflowButton_.setTooltip("Open labelled shell actions that do not fit at this width.");

    comparisonButton_.setClickingTogglesState(true);
    processingButton_.setClickingTogglesState(true);

    previousPresetButton_.onClick = [this] { emit(ShellAction::previousPreset); };
    nextPresetButton_.onClick = [this] { emit(ShellAction::nextPreset); };
    savePresetButton_.onClick = [this] { emit(ShellAction::savePreset); };
    undoButton_.onClick = [this] { emit(ShellAction::undo); };
    redoButton_.onClick = [this] { emit(ShellAction::redo); };
    comparisonButton_.onClick = [this] { emit(ShellAction::toggleComparison); };
    processingButton_.onClick = [this] { emit(ShellAction::toggleProcessing); };
    helpButton_.onClick = [this] { emit(ShellAction::help); };
    scaleButton_.onClick = [this] { emit(ShellAction::cycleScale); };
    overflowButton_.onClick = [this] { showOverflowMenu(); };

    statusLabel_.setJustificationType(juce::Justification::centred);
    statusLabel_.setFont(font(10.5f, true));
    statusLabel_.setMinimumHorizontalScale(0.8f);

    for (auto* component : std::array<juce::Component*, 13> {
             &identityLabel_,
             &presetSelector_,
             &previousPresetButton_,
             &nextPresetButton_,
             &savePresetButton_,
             &undoButton_,
             &redoButton_,
             &comparisonButton_,
             &processingButton_,
             &helpButton_,
             &scaleButton_,
             &overflowButton_,
             &statusLabel_ }) {
        addAndMakeVisible(*component);
    }
}

void SuiteShell::setPresentation(preview::ShellPresentation presentation)
{
    presentation_ = std::move(presentation);

    identityLabel_.setText(
        "EHVOTA / " + juce::String(presentation_.productName),
        juce::dontSendNotification);
    identityLabel_.setDescription(
        "Ehvota " + juce::String(presentation_.productName));

    presetSelector_.clear(juce::dontSendNotification);
    presetSelector_.addItem(juce::String(presentation_.presetName), 1);
    if (presentation_.presetName != "Default")
        presetSelector_.addItem("Default", 2);
    presetSelector_.addItem("Review A", 3);
    presetSelector_.addItem("Review B", 4);
    presetSelector_.setSelectedId(1, juce::dontSendNotification);

    undoButton_.setEnabled(presentation_.canUndo);
    redoButton_.setEnabled(presentation_.canRedo);
    comparisonButton_.setToggleState(
        presentation_.comparisonB,
        juce::dontSendNotification);
    comparisonButton_.setButtonText(presentation_.comparisonB ? "B" : "A");
    comparisonButton_.setDescription(
        presentation_.comparisonB ? "Comparison state B" : "Comparison state A");

    processingButton_.setButtonText(
        juce::String(presentation_.processingActionLabel));
    processingButton_.setToggleState(
        presentation_.processingActive,
        juce::dontSendNotification);
    processingButton_.setEnabled(presentation_.processingActionEnabled);
    scaleButton_.setButtonText(juce::String(presentation_.scalePercent) + "%");

    statusLabel_.setText(
        juce::String(presentation_.statusText),
        juce::dontSendNotification);
    statusLabel_.setTooltip(juce::String(presentation_.statusDetail));
    statusLabel_.setDescription(
        juce::String(presentation_.statusText) + ". "
        + juce::String(presentation_.statusDetail));
    statusLabel_.setColour(
        juce::Label::textColourId,
        juce::Colour(
            presentation_.statusIsWarning ? colour::caution : colour::positive));

    setTitle("Ehvota " + juce::String(presentation_.productName) + " shared shell");
    setDescription(
        "Preset " + juce::String(presentation_.presetName)
        + ". Comparison " + (presentation_.comparisonB ? "B" : "A")
        + ". Status " + juce::String(presentation_.statusText)
        + (presentation_.showOutputMeter
                ? ". Output " + juce::String(presentation_.outputLevelDb, 1) + " decibels"
                : juce::String {}));

    resized();
    repaint();
}

juce::String SuiteShell::selectedPresetName() const
{
    return presetSelector_.getText();
}

void SuiteShell::showPresetMenu()
{
    presetSelector_.showPopup();
}

void SuiteShell::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    graphics.setColour(juce::Colour(colour::base));
    graphics.fillRoundedRectangle(bounds, 7.0f);
    graphics.setColour(juce::Colour(colour::borderSubtle));
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 7.0f, 1.0f);

    if (!meterBounds_.isEmpty() && presentation_.showOutputMeter) {
        const auto meter = meterBounds_.toFloat();
        graphics.setColour(juce::Colour(colour::overlay));
        graphics.fillRoundedRectangle(meter, 2.0f);

        const auto normalized = juce::jlimit(
            0.0,
            1.0,
            (presentation_.outputLevelDb + 60.0) / 60.0);
        auto level = meter;
        level.removeFromTop(
            level.getHeight() * static_cast<float>(1.0 - normalized));
        graphics.setColour(
            juce::Colour(
                presentation_.outputLevelDb > -3.0 ? colour::caution : colour::positive));
        graphics.fillRoundedRectangle(level, 2.0f);
    }
}

void SuiteShell::resized()
{
    if (getWidth() <= 0)
        return;

    density_ = preview::classifyShellDensity(getWidth());
    hideAllActionComponents();

    auto content = getLocalBounds().reduced(8, 6);
    meterBounds_ = {};
    if (presentation_.showOutputMeter) {
        meterBounds_ = content.removeFromRight(7);
        content.removeFromRight(6);
    }

    statusLabel_.setBounds(content.removeFromRight(
        density_ == preview::ShellDensity::wide ? 100 : 86));
    statusLabel_.setVisible(true);
    content.removeFromRight(controlGap);

    const auto compact = density_ == preview::ShellDensity::compact;
    const auto wide = density_ == preview::ShellDensity::wide;
    placeFromLeft(content, identityLabel_, wide ? 112 : 86);
    placeFromLeft(content, presetSelector_, wide ? 176 : 138);

    if (wide) {
        placeFromLeft(content, previousPresetButton_, 46);
        placeFromLeft(content, nextPresetButton_, 46);
        placeFromLeft(content, savePresetButton_, 50);
        placeFromLeft(content, undoButton_, 50);
        placeFromLeft(content, redoButton_, 50);
    }
    else if (!compact) {
        placeFromLeft(content, undoButton_, 50);
        placeFromLeft(content, redoButton_, 50);
    }

    placeFromLeft(content, comparisonButton_, 42);
    if (presentation_.showProcessingAction)
        placeFromLeft(content, processingButton_, wide ? 78 : 68);
    placeFromLeft(content, helpButton_, 46);
    placeFromLeft(content, scaleButton_, wide ? 62 : 56);

    if (!wide)
        placeFromLeft(content, overflowButton_, 54);
}

std::unique_ptr<juce::AccessibilityHandler> SuiteShell::createAccessibilityHandler()
{
    return std::make_unique<juce::AccessibilityHandler>(
        *this,
        juce::AccessibilityRole::group);
}

void SuiteShell::emit(const ShellAction action)
{
    if (onAction)
        onAction(action);
}

void SuiteShell::showOverflowMenu()
{
    juce::PopupMenu menu;
    menu.addItem(1, "Previous preset");
    menu.addItem(2, "Next preset");
    menu.addItem(3, "Save preset");
    if (density_ == preview::ShellDensity::compact) {
        menu.addSeparator();
        menu.addItem(4, "Undo", presentation_.canUndo);
        menu.addItem(5, "Redo", presentation_.canRedo);
    }

    const juce::Component::SafePointer<SuiteShell> safeThis(this);
    menu.showMenuAsync(
        juce::PopupMenu::Options().withTargetComponent(&overflowButton_),
        [safeThis](const int result) {
            if (safeThis == nullptr)
                return;
            switch (result) {
            case 1:
                safeThis->emit(ShellAction::previousPreset);
                break;
            case 2:
                safeThis->emit(ShellAction::nextPreset);
                break;
            case 3:
                safeThis->emit(ShellAction::savePreset);
                break;
            case 4:
                safeThis->emit(ShellAction::undo);
                break;
            case 5:
                safeThis->emit(ShellAction::redo);
                break;
            default:
                break;
            }
        });
}

void SuiteShell::hideAllActionComponents()
{
    for (auto* component : std::array<juce::Component*, 13> {
             &identityLabel_,
             &presetSelector_,
             &previousPresetButton_,
             &nextPresetButton_,
             &savePresetButton_,
             &undoButton_,
             &redoButton_,
             &comparisonButton_,
             &processingButton_,
             &helpButton_,
             &scaleButton_,
             &overflowButton_,
             &statusLabel_ }) {
        component->setVisible(false);
        component->setBounds({});
    }
}

} // namespace ehvota::ui
