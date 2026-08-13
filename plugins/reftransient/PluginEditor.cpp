#include "PluginEditor.h"

#include <cmath>

namespace ehvota::reftransient {
namespace {

const auto canvas = juce::Colour(0xff0b0f14);
const auto panel = juce::Colour(0xff10161d);
const auto border = juce::Colour(0xff26313b);
const auto text = juce::Colour(0xffe6edf3);
const auto secondary = juce::Colour(0xff8e9aa6);
const auto cyan = juce::Colour(0xff65d6ff);
const auto amber = juce::Colour(0xffe8a85b);

void configureKnob(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 84, 22);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::rotarySliderFillColourId, cyan);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, border);
    slider.setColour(juce::Slider::textBoxTextColourId, text);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, canvas);
    slider.setColour(juce::Slider::textBoxOutlineColourId, border);
}

} // namespace

RefTransientAudioProcessorEditor::RefTransientAudioProcessorEditor(RefTransientAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processor_(processor)
{
    const std::array<const char*, 4> ids { "attack", "sustain", "sensitivity", "output" };
    const std::array<juce::String, 4> suffixes { " %", " %", " %", " dB" };
    for (std::size_t i = 0; i < controls_.size(); ++i) {
        configureKnob(controls_[i], suffixes[i]);
        addAndMakeVisible(controls_[i]);
        attachments_[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor_.parameters(), ids[i], controls_[i]);
    }

    activityLabel_.setJustificationType(juce::Justification::centred);
    activityLabel_.setColour(juce::Label::textColourId, amber);
    activityLabel_.setFont(juce::FontOptions(20.0f).withStyle("Bold"));
    addAndMakeVisible(activityLabel_);

    setResizable(true, true);
    setResizeLimits(560, 380, 1100, 760);
    setSize(760, 520);
    startTimerHz(20);
}

void RefTransientAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(canvas);
    g.setColour(text);
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawText("Ehvota RefTransient", 20, 14, getWidth() - 40, 30, juce::Justification::centredLeft);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("STEREO-LINKED TRANSIENT SHAPING · IMAGE-SAFE DETECTION", 20, 44, getWidth() - 40, 18, juce::Justification::centredLeft);

    auto activity = juce::Rectangle<int>(20, 82, getWidth() - 40, 104);
    g.setColour(panel);
    g.fillRoundedRectangle(activity.toFloat(), 8.0f);
    g.setColour(border);
    g.drawRoundedRectangle(activity.toFloat().reduced(0.5f), 8.0f, 1.0f);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText("TRANSIENT ACTIVITY", activity.getX() + 12, activity.getY() + 10, activity.getWidth() - 24, 18, juce::Justification::centred);

    const std::array<const char*, 4> labels { "ATTACK", "SUSTAIN", "SENSITIVITY", "OUTPUT" };
    const auto controlsTop = 236;
    const auto width = (getWidth() - 40) / 4;
    for (std::size_t i = 0; i < labels.size(); ++i) {
        g.setColour(secondary);
        g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
        g.drawText(labels[i], 20 + static_cast<int>(i) * width, controlsTop - 22, width, 18, juce::Justification::centred);
    }

    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f));
    g.drawFittedText("Attack and sustain are derived from the difference between fast and slow envelopes. The control signal is linked across channels so transient shaping does not create accidental stereo image shifts.", 26, getHeight() - 62, getWidth() - 52, 38, juce::Justification::centred, 2);
}

void RefTransientAudioProcessorEditor::resized()
{
    activityLabel_.setBounds(70, 116, getWidth() - 140, 42);
    auto controlsArea = juce::Rectangle<int>(20, 236, getWidth() - 40, juce::jmax(120, getHeight() - 330));
    const auto width = controlsArea.getWidth() / 4;
    for (auto& control : controls_)
        control.setBounds(controlsArea.removeFromLeft(width).reduced(8));
}

void RefTransientAudioProcessorEditor::timerCallback()
{
    const auto activity = processor_.transientActivity();
    const auto label = activity >= 0.0f ? "ATTACK +" : "SUSTAIN / DECAY ";
    activityLabel_.setText(label + juce::String(std::abs(activity) * 100.0f, 1) + " %", juce::dontSendNotification);
}

} // namespace ehvota::reftransient
