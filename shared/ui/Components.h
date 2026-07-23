#pragma once

#include "shared/viewmodels/PreviewScenario.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace ehvota::ui {

class StatusBadge final : public juce::Component {
public:
    explicit StatusBadge(preview::Badge badge);

    void paint(juce::Graphics&) override;
    [[nodiscard]] int idealWidth() const;
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

private:
    preview::Badge badge_;
};

class MetricPanel final : public juce::Component {
public:
    explicit MetricPanel(preview::Panel panel);

    void paint(juce::Graphics&) override;
    [[nodiscard]] int preferredHeight() const;
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

private:
    preview::Panel panel_;
};

class SignalGraph final : public juce::Component {
public:
    void setScenario(std::string scenarioId);
    void paint(juce::Graphics&) override;
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

private:
    std::string scenarioId_ { "compare-chorus" };
};

class TimingReadout final : public juce::Component {
public:
    void setValues(juce::String exact, juce::String secondary);
    void paint(juce::Graphics&) override;
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

private:
    juce::String exact_ { "93.750 ms" };
    juce::String secondary_ { "Rounded 94 ms · 4,500 samples" };
};

class ProgressTimeline final : public juce::Component {
public:
    void setState(juce::String title, juce::String currentStep, double progress);
    void paint(juce::Graphics&) override;
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

private:
    juce::String title_ { "Local stem preparation" };
    juce::String currentStep_ { "Waiting" };
    double progress_ {};
};

} // namespace ehvota::ui
