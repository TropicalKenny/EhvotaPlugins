#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace ehvota::previewapp {

class JourneyProgressComponent final : public juce::Component {
public:
    JourneyProgressComponent();

    void setStep(
        int journeyNumber,
        juce::String journeyTitle,
        int currentStep,
        int totalSteps,
        juce::String stepTitle,
        juce::String instruction);
    void paint(juce::Graphics&) override;
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

private:
    int journeyNumber_ {};
    int currentStep_ {};
    int totalSteps_ {};
    juce::String journeyTitle_;
    juce::String stepTitle_;
    juce::String instruction_;
};

} // namespace ehvota::previewapp
