#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace ehvota::previewapp {

class ApprovedSourcesPreview final : public juce::Component {
public:
    ApprovedSourcesPreview();
    ~ApprovedSourcesPreview() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::TextButton validateButton_ { "VALIDATE MAPPING" };
    juce::TextButton refreshButton_ { "REFRESH LIVE SET" };
    juce::TextButton saveMappingButton_ { "SAVE MAPPING" };
    juce::TooltipWindow tooltipWindow_ { this, 500 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ApprovedSourcesPreview)
};

} // namespace ehvota::previewapp
