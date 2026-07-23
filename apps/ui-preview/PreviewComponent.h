#pragma once

#include "shared/ui/Components.h"
#include "shared/viewmodels/PreviewScenario.h"

#include <juce_gui_extra/juce_gui_extra.h>

#include <memory>
#include <vector>

namespace ehvota::previewapp {

class PreviewComponent final : public juce::Component {
public:
    PreviewComponent();
    ~PreviewComponent() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;

private:
    void loadScenario(int scenarioIndex);
    void rebuildScenarioComponents();
    void updateNavigation();
    void setTransientStatus(juce::String status);
    [[nodiscard]] bool isCompact() const;
    [[nodiscard]] bool isWide() const;

    juce::ComboBox scenarioSelector_;
    juce::TextButton menuButton_ { "VIEW" };
    juce::TextButton mixButton_ { "MIX" };
    juce::TextButton referenceButton_ { "REFERENCE" };
    juce::TextButton loudnessMatchButton_ { "MATCH" };
    juce::TextButton captureButton_ { "CAPTURE" };
    juce::TextButton blindButton_ { "BLIND" };
    juce::TextButton primaryActionButton_ { "OPEN" };
    juce::TextButton secondaryActionButton_ { "DETAILS" };
    juce::TooltipWindow tooltipWindow_ { this, 550 };

    std::vector<std::unique_ptr<juce::TextButton>> navigationButtons_;
    std::vector<std::unique_ptr<ui::StatusBadge>> badges_;
    juce::Component panelsContent_;
    juce::Viewport panelsViewport_;
    std::vector<std::unique_ptr<ui::MetricPanel>> panels_;
    ui::SignalGraph signalGraph_;
    ui::TimingReadout timingReadout_;
    ui::ProgressTimeline progressTimeline_;
    std::vector<juce::String> currentViewNames_;

    const preview::Scenario* scenario_ { nullptr };
    int scenarioIndex_ { 0 };
    juce::String transientStatus_ { "Deterministic preview data · No audio is processed" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewComponent)
};

} // namespace ehvota::previewapp
