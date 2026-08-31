#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace ehvota::previewapp {

class ApprovedRefTimePreview final : public juce::Component {
public:
    ApprovedRefTimePreview();
    ~ApprovedRefTimePreview() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::ComboBox divisionSelector_;
    juce::ComboBox modifierSelector_;
    juce::ComboBox outputModeSelector_;
    juce::TextButton copyExactButton_ { "COPY EXACT MS" };
    juce::TextButton copyRoundedButton_ { "COPY ROUNDED" };
    juce::TextButton copyNumberButton_ { "COPY NUMBER" };
    juce::TextButton copySamplesButton_ { "COPY SAMPLES" };
    juce::TooltipWindow tooltipWindow_ { this, 500 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ApprovedRefTimePreview)
};

} // namespace ehvota::previewapp
