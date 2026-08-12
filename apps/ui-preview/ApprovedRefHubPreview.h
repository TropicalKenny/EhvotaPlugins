#pragma once

#include "shared/viewmodels/LevelMatchState.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace ehvota::previewapp {

class ApprovedRefHubPreview final : public juce::Component {
public:
    ApprovedRefHubPreview();
    ~ApprovedRefHubPreview() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void configureButton(juce::TextButton& button, const juce::String& tooltip);
    void updateLevelMatchSummary();

    juce::ComboBox referenceSelector_;
    juce::ComboBox sectionSelector_;
    juce::TextButton mixButton_ { "MIX" };
    juce::TextButton referenceButton_ { "REF" };
    juce::TextButton blindButton_ { "BLIND" };
    juce::TextButton captureButton_ { "CAPTURE" };
    juce::TextButton inspectorButton_ { "INSPECTOR" };
    juce::TextButton listenRangeButton_ { "LISTEN RANGE" };
    juce::TextButton compareButton_ { "COMPARE A/B" };
    juce::TextButton createProposalButton_ { "CREATE PROPOSAL" };
    juce::ComboBox levelMatchMode_;
    juce::Label levelMatchSummary_;
    juce::TooltipWindow tooltipWindow_ { this, 500 };

    ehvota::preview::LevelMatchState levelMatchState_;
    bool inspectorOpen_ { true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ApprovedRefHubPreview)
};

} // namespace ehvota::previewapp
