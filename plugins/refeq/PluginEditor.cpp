#include "PluginEditor.h"

namespace ehvota::refeq {
namespace {
const auto canvas = juce::Colour(0xff0b0f14);
const auto panel = juce::Colour(0xff10161d);
const auto border = juce::Colour(0xff26313b);
const auto text = juce::Colour(0xffe6edf3);
const auto secondary = juce::Colour(0xff8e9aa6);
const auto cyan = juce::Colour(0xff65d6ff);
}

RefEQAudioProcessorEditor::RefEQAudioProcessorEditor(RefEQAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processor_(processor)
{
    mode_.addItem("Stereo", 1);
    mode_.addItem("Mid", 2);
    mode_.addItem("Side", 3);
    mode_.setColour(juce::ComboBox::backgroundColourId, panel);
    mode_.setColour(juce::ComboBox::textColourId, text);
    mode_.setColour(juce::ComboBox::outlineColourId, border);
    addAndMakeVisible(mode_);
    modeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor_.parameters(), "mode", mode_);

    configureToggle(highPassEnabled_);
    configureToggle(lowPassEnabled_);
    addAndMakeVisible(highPassEnabled_);
    addAndMakeVisible(lowPassEnabled_);
    highPassEnabledAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor_.parameters(), "hpEnabled", highPassEnabled_);
    lowPassEnabledAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor_.parameters(), "lpEnabled", lowPassEnabled_);

    configureKnob(highPass_, " Hz");
    configureKnob(lowPass_, " Hz");
    configureKnob(output_, " dB");
    addAndMakeVisible(highPass_);
    addAndMakeVisible(lowPass_);
    addAndMakeVisible(output_);
    highPassAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor_.parameters(), "hpFreq", highPass_);
    lowPassAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor_.parameters(), "lpFreq", lowPass_);
    outputAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor_.parameters(), "output", output_);

    for (std::size_t i = 0; i < bands_.size(); ++i) {
        auto& band = bands_[i];
        configureKnob(band.frequency, " Hz");
        configureKnob(band.gain, " dB");
        configureKnob(band.q, " Q");
        addAndMakeVisible(band.frequency);
        addAndMakeVisible(band.gain);
        addAndMakeVisible(band.q);
        const auto prefix = "b" + juce::String(static_cast<int>(i + 1));
        band.frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor_.parameters(), prefix + "Freq", band.frequency);
        band.gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor_.parameters(), prefix + "Gain", band.gain);
        band.qAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor_.parameters(), prefix + "Q", band.q);
    }

    setResizable(true, true);
    setResizeLimits(820, 560, 1500, 980);
    setSize(1080, 720);
}

void RefEQAudioProcessorEditor::configureKnob(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 78, 19);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::rotarySliderFillColourId, cyan);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a3540));
    slider.setColour(juce::Slider::textBoxTextColourId, text);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void RefEQAudioProcessorEditor::configureToggle(juce::ToggleButton& button)
{
    button.setColour(juce::ToggleButton::textColourId, text);
    button.setColour(juce::ToggleButton::tickColourId, cyan);
    button.setColour(juce::ToggleButton::tickDisabledColourId, secondary);
}

void RefEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(canvas);
    g.setColour(text);
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawText("Ehvota RefEQ", 20, 14, getWidth() - 40, 30, juce::Justification::centredLeft);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("REFERENCE-LINKED 6-BAND EQ · STEREO / MID / SIDE · EXACT NEUTRAL PATH", 20, 44, getWidth() - 40, 18, juce::Justification::centredLeft);

    auto graph = juce::Rectangle<int>(20, 104, getWidth() - 40, juce::jmax(150, getHeight() / 4));
    g.setColour(panel);
    g.fillRoundedRectangle(graph.toFloat(), 8.0f);
    g.setColour(border);
    g.drawRoundedRectangle(graph.toFloat().reduced(0.5f), 8.0f, 1.0f);
    for (int i = 1; i < 10; ++i) {
        const auto x = graph.getX() + graph.getWidth() * i / 10;
        g.drawVerticalLine(x, static_cast<float>(graph.getY()), static_cast<float>(graph.getBottom()));
    }
    g.setColour(cyan);
    g.drawHorizontalLine(graph.getCentreY(), static_cast<float>(graph.getX() + 10), static_cast<float>(graph.getRight() - 10));

    g.setColour(secondary);
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    const auto controlTop = graph.getBottom() + 20;
    g.drawText("FILTERS", 20, controlTop, 180, 18, juce::Justification::centredLeft);
    g.drawText("OUTPUT", getWidth() - 120, controlTop, 90, 18, juce::Justification::centred);

    const auto cardsTop = controlTop + 82;
    const auto available = getWidth() - 40;
    const auto cardWidth = (available - 20) / 3;
    for (int i = 0; i < 6; ++i) {
        const int row = i / 3;
        const int column = i % 3;
        const auto card = juce::Rectangle<int>(20 + column * (cardWidth + 10), cardsTop + row * 142, cardWidth, 132);
        g.setColour(panel);
        g.fillRoundedRectangle(card.toFloat(), 7.0f);
        g.setColour(border);
        g.drawRoundedRectangle(card.toFloat().reduced(0.5f), 7.0f, 1.0f);
        g.setColour(secondary);
        g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
        g.drawText("BAND " + juce::String(i + 1), card.getX() + 10, card.getY() + 7, card.getWidth() - 20, 16, juce::Justification::centredLeft);
    }
}

void RefEQAudioProcessorEditor::resized()
{
    mode_.setBounds(getWidth() - 178, 18, 150, 30);
    auto graph = juce::Rectangle<int>(20, 104, getWidth() - 40, juce::jmax(150, getHeight() / 4));
    const auto controlTop = graph.getBottom() + 20;

    highPassEnabled_.setBounds(24, controlTop + 23, 46, 28);
    highPass_.setBounds(68, controlTop + 12, 100, 64);
    lowPassEnabled_.setBounds(178, controlTop + 23, 46, 28);
    lowPass_.setBounds(220, controlTop + 12, 100, 64);
    output_.setBounds(getWidth() - 116, controlTop + 10, 92, 68);

    const auto cardsTop = controlTop + 82;
    const auto available = getWidth() - 40;
    const auto cardWidth = (available - 20) / 3;
    for (int i = 0; i < 6; ++i) {
        const int row = i / 3;
        const int column = i % 3;
        auto card = juce::Rectangle<int>(20 + column * (cardWidth + 10), cardsTop + row * 142, cardWidth, 132).reduced(8, 22);
        auto& band = bands_[static_cast<std::size_t>(i)];
        const auto knobWidth = card.getWidth() / 3;
        band.frequency.setBounds(card.removeFromLeft(knobWidth).reduced(2));
        band.gain.setBounds(card.removeFromLeft(knobWidth).reduced(2));
        band.q.setBounds(card.reduced(2));
    }
}

} // namespace ehvota::refeq
