#include "PluginEditor.h"

namespace ehvota::refcolor {
namespace {

const auto canvas = juce::Colour(0xff090d12);
const auto panel = juce::Colour(0xff10161d);
const auto border = juce::Colour(0xff26313b);
const auto text = juce::Colour(0xffe8edf2);
const auto secondary = juce::Colour(0xff8e9aa6);
const auto amber = juce::Colour(0xffe8a85b);

void configureKnob(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 78, 22);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::rotarySliderFillColourId, amber);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, border);
    slider.setColour(juce::Slider::textBoxTextColourId, text);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, canvas);
    slider.setColour(juce::Slider::textBoxOutlineColourId, border);
}

void configureLabel(juce::Label& label, const juce::String& value)
{
    label.setText(value, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, secondary);
}

} // namespace

RefColorAudioProcessorEditor::RefColorAudioProcessorEditor(RefColorAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processor_(processor)
{
    mode_.addItem("Console", 1);
    mode_.addItem("Tape", 2);
    mode_.addItem("Transformer", 3);
    addAndMakeVisible(mode_);

    configureKnob(amount_, "%");
    configureKnob(drive_, " dB");
    configureKnob(asymmetry_, "");
    configureKnob(output_, " dB");

    for (auto* slider : { &amount_, &drive_, &asymmetry_, &output_ })
        addAndMakeVisible(*slider);

    configureLabel(amountLabel_, "AMOUNT");
    configureLabel(driveLabel_, "DRIVE");
    configureLabel(asymmetryLabel_, "ASYMMETRY");
    configureLabel(outputLabel_, "OUTPUT");
    for (auto* label : { &amountLabel_, &driveLabel_, &asymmetryLabel_, &outputLabel_ })
        addAndMakeVisible(*label);

    qualityLabel_.setText("4× HQ OVERSAMPLING · CALIBRATED GAIN · DC PROTECTED", juce::dontSendNotification);
    qualityLabel_.setJustificationType(juce::Justification::centred);
    qualityLabel_.setColour(juce::Label::textColourId, secondary);
    addAndMakeVisible(qualityLabel_);

    auto& state = processor_.parameters();
    modeAttachment_ = std::make_unique<ComboAttachment>(state, "mode", mode_);
    amountAttachment_ = std::make_unique<SliderAttachment>(state, "amount", amount_);
    driveAttachment_ = std::make_unique<SliderAttachment>(state, "drive", drive_);
    asymmetryAttachment_ = std::make_unique<SliderAttachment>(state, "asymmetry", asymmetry_);
    outputAttachment_ = std::make_unique<SliderAttachment>(state, "output", output_);

    setResizable(true, true);
    setResizeLimits(560, 380, 1100, 760);
    setSize(760, 520);
}

void RefColorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(canvas);
    auto bounds = getLocalBounds().reduced(16);

    g.setColour(text);
    g.setFont(juce::FontOptions(24.0f).withStyle("Bold"));
    g.drawText("Ehvota RefColor", bounds.removeFromTop(38), juce::Justification::centredLeft);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("SUBTLE ANALOGUE CHARACTER · DEFAULT INSERTION IS INTENTIONALLY GENTLE",
        bounds.removeFromTop(22), juce::Justification::centredLeft);

    auto card = bounds.reduced(0, 14);
    g.setColour(panel);
    g.fillRoundedRectangle(card.toFloat(), 10.0f);
    g.setColour(border);
    g.drawRoundedRectangle(card.toFloat().reduced(0.5f), 10.0f, 1.0f);

    auto profileArea = card.reduced(18).removeFromTop(72);
    g.setColour(amber);
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("CHARACTER PROFILE", profileArea.removeFromTop(18), juce::Justification::centredLeft);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("Console = restrained saturation · Tape = rounder peaks · Transformer = more even-harmonic bias",
        profileArea, juce::Justification::centredLeft);
}

void RefColorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(16);
    bounds.removeFromTop(60);
    auto card = bounds.reduced(0, 14).reduced(18);

    auto profile = card.removeFromTop(76);
    mode_.setBounds(profile.removeFromRight(190).reduced(0, 14));

    card.removeFromTop(8);
    auto controls = card.removeFromTop(250);
    const auto width = controls.getWidth() / 4;
    auto place = [&](juce::Slider& slider, juce::Label& label) {
        auto cell = controls.removeFromLeft(width);
        label.setBounds(cell.removeFromTop(24));
        slider.setBounds(cell.reduced(8));
    };
    place(amount_, amountLabel_);
    place(drive_, driveLabel_);
    place(asymmetry_, asymmetryLabel_);
    place(output_, outputLabel_);

    qualityLabel_.setBounds(card.removeFromBottom(34));
}

} // namespace ehvota::refcolor
