#include "PluginEditor.h"

namespace ehvota::refcomp {

RefCompAudioProcessorEditor::RefCompAudioProcessorEditor(RefCompAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processor_(processor)
{
    const std::array<const char*, 7> ids { "threshold", "ratio", "attack", "release", "knee", "makeup", "mix" };
    const std::array<juce::String, 7> suffixes { " dB", ":1", " ms", " ms", " dB", " dB", " %" };

    for (std::size_t i = 0; i < controls_.size(); ++i) {
        configureKnob(controls_[i], suffixes[i]);
        addAndMakeVisible(controls_[i]);
        attachments_[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor_.parameters(), ids[i], controls_[i]);
    }

    gainReductionLabel_.setJustificationType(juce::Justification::centred);
    gainReductionLabel_.setColour(juce::Label::textColourId, juce::Colour(0xffe8a85b));
    gainReductionLabel_.setFont(juce::FontOptions(24.0f).withStyle("Bold"));
    addAndMakeVisible(gainReductionLabel_);

    setResizable(true, true);
    setResizeLimits(700, 460, 1300, 850);
    setSize(920, 620);
    startTimerHz(20);
}

void RefCompAudioProcessorEditor::configureKnob(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 84, 20);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff65d6ff));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a3540));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe6edf3));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void RefCompAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0b0f14));
    g.setColour(juce::Colour(0xffe6edf3));
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawText("Ehvota RefComp", 20, 14, getWidth() - 40, 30, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff7f8b98));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("REFERENCE-LINKED DYNAMICS · STEREO-LINKED DETECTOR", 20, 44, getWidth() - 40, 18, juce::Justification::centredLeft);

    auto meter = juce::Rectangle<int>(20, 82, getWidth() - 40, 150);
    g.setColour(juce::Colour(0xff10161d));
    g.fillRoundedRectangle(meter.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xff26313b));
    g.drawRoundedRectangle(meter.toFloat().reduced(0.5f), 8.0f, 1.0f);
    g.setColour(juce::Colour(0xff8e9aa6));
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("GAIN REDUCTION", meter.getX() + 18, meter.getY() + 16, meter.getWidth() - 36, 20, juce::Justification::centred);

    const std::array<const char*, 7> labels { "THRESHOLD", "RATIO", "ATTACK", "RELEASE", "KNEE", "MAKEUP", "MIX" };
    const auto controlsTop = 282;
    const auto width = (getWidth() - 40) / static_cast<int>(labels.size());
    for (std::size_t i = 0; i < labels.size(); ++i) {
        g.setColour(juce::Colour(0xff8e9aa6));
        g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
        g.drawText(labels[i], 20 + static_cast<int>(i) * width, controlsTop - 22, width, 18, juce::Justification::centred);
    }

    g.setColour(juce::Colour(0xff7f8b98));
    g.setFont(juce::FontOptions(10.0f));
    g.drawFittedText("Neutral by default. Reference-driven changes arrive as bounded proposals and require explicit approval before they can alter these parameters.", 26, getHeight() - 62, getWidth() - 52, 36, juce::Justification::centred, 2);
}

void RefCompAudioProcessorEditor::resized()
{
    gainReductionLabel_.setBounds(80, 126, getWidth() - 160, 54);

    auto controlsArea = juce::Rectangle<int>(20, 282, getWidth() - 40, juce::jmax(120, getHeight() - 370));
    const auto width = controlsArea.getWidth() / static_cast<int>(controls_.size());
    for (auto& control : controls_)
        control.setBounds(controlsArea.removeFromLeft(width).reduced(5));
}

void RefCompAudioProcessorEditor::timerCallback()
{
    gainReductionLabel_.setText(
        "−" + juce::String(processor_.gainReductionDb(), 2) + " dB",
        juce::dontSendNotification);
}

} // namespace ehvota::refcomp
