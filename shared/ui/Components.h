#pragma once

#include "shared/viewmodels/PreviewScenario.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace ehvota::ui {

class StatusBadge final : public juce::Component {
public:
    explicit StatusBadge(preview::Badge badge);

    void paint(juce::Graphics&) override;
    [[nodiscard]] int idealWidth() const;

private:
    preview::Badge badge_;
};

class MetricPanel final : public juce::Component {
public:
    explicit MetricPanel(preview::Panel panel);

    void paint(juce::Graphics&) override;
    [[nodiscard]] int preferredHeight() const;

private:
    preview::Panel panel_;
};

class SignalGraph final : public juce::Component {
public:
    void setScenario(std::string scenarioId);
    void paint(juce::Graphics&) override;

private:
    std::string scenarioId_ { "compare-chorus" };
};

} // namespace ehvota::ui
