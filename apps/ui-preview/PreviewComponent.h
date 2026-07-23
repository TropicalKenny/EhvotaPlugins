#pragma once

#include "JourneyProgressComponent.h"
#include "shared/ui/Components.h"
#include "shared/viewmodels/JourneyReplay.h"
#include "shared/viewmodels/PreviewScenario.h"

#include <juce_gui_extra/juce_gui_extra.h>

#include <memory>
#include <string_view>
#include <vector>

namespace ehvota::previewapp {

class PreviewComponent final : public juce::Component {
public:
    PreviewComponent();
    ~PreviewComponent() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;

    [[nodiscard]] bool selectScenarioById(std::string_view scenarioId);
    [[nodiscard]] bool selectJourneyStep(std::size_t oneBasedStep);

private:
    void loadScenario(int scenarioIndex);
    void rebuildScenarioComponents();
    void updateNavigation();
    void updateJourneyReplay();
    void moveJourneyStep(bool forward);
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
    juce::TextButton journeyBackButton_ { "BACK" };
    juce::TextButton journeyNextButton_ { "NEXT" };
    juce::TooltipWindow tooltipWindow_ { this, 550 };

    std::vector<std::unique_ptr<juce::TextButton>> navigationButtons_;
    std::vector<std::unique_ptr<ui::StatusBadge>> badges_;
    juce::Component panelsContent_;
    juce::Viewport panelsViewport_;
    std::vector<std::unique_ptr<ui::MetricPanel>> panels_;
    ui::SignalGraph signalGraph_;
    ui::TimingReadout timingReadout_;
    ui::ProgressTimeline progressTimeline_;
    JourneyProgressComponent journeyProgress_;
    std::vector<juce::String> currentViewNames_;

    const preview::Scenario* scenario_ { nullptr };
    preview::JourneyReplay journeyReplay_;
    int scenarioIndex_ { 0 };
    juce::String transientStatus_ { "Deterministic preview data · No audio is processed" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewComponent)
};

} // namespace ehvota::previewapp
