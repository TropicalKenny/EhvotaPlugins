#pragma once

#include "shared/viewmodels/ShellPresentation.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace ehvota::ui {

enum class ShellAction {
    selectPreset,
    previousPreset,
    nextPreset,
    savePreset,
    undo,
    redo,
    toggleComparison,
    toggleProcessing,
    help,
    cycleScale
};

class SuiteShell final : public juce::Component {
public:
    SuiteShell();

    void setPresentation(preview::ShellPresentation presentation);
    [[nodiscard]] juce::String selectedPresetName() const;
    void showPresetMenu();

    void paint(juce::Graphics&) override;
    void resized() override;
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

    std::function<void(ShellAction)> onAction;

private:
    void emit(ShellAction action);
    void showOverflowMenu();
    void hideAllActionComponents();

    preview::ShellPresentation presentation_;
    preview::ShellDensity density_ { preview::ShellDensity::compact };

    juce::Label identityLabel_;
    juce::ComboBox presetSelector_;
    juce::TextButton previousPresetButton_ { "PREV" };
    juce::TextButton nextPresetButton_ { "NEXT" };
    juce::TextButton savePresetButton_ { "SAVE" };
    juce::TextButton undoButton_ { "UNDO" };
    juce::TextButton redoButton_ { "REDO" };
    juce::TextButton comparisonButton_ { "A" };
    juce::TextButton processingButton_ { "BYPASS" };
    juce::TextButton helpButton_ { "HELP" };
    juce::TextButton scaleButton_ { "100%" };
    juce::TextButton overflowButton_ { "MORE" };
    juce::Label statusLabel_;
    juce::Rectangle<int> meterBounds_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SuiteShell)
};

} // namespace ehvota::ui
