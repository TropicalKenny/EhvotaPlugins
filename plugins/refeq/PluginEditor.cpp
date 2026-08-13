#include "PluginEditor.h"

namespace ehvota::refeq {

RefEQAudioProcessorEditor::RefEQAudioProcessorEditor(RefEQAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processor_(processor)
{
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
    setResizeLimits(720, 480, 1400, 900);
    setSize(920, 620);
}

void RefEQAudioProcessorEditor::configureKnob(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 82, 20);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff65d6ff));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a3540));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe6edf3));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void RefEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0b0f14));
    g.setColour(juce::Colour(0xffe6edf3));
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawText("Ehvota RefEQ", 20, 14, getWidth() - 40, 30, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff7f8b98));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("REFERENCE-LINKED PARAMETRIC EQ · NEUTRAL PATH", 20, 44, getWidth() - 40, 18, juce::Justification::centredLeft);

    auto graph = juce::Rectangle<int>(20, 76, getWidth() - 40, juce::jmax(150, getHeight() / 3));
    g.setColour(juce::Colour(0xff10161d));
    g.fillRoundedRectangle(graph.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xff26313b));
    g.drawRoundedRectangle(graph.toFloat().reduced(0.5f), 8.0f, 1.0f);
    for (int i = 1; i < 10; ++i) {
        const auto x = graph.getX() + graph.getWidth() * i / 10;
        g.drawVerticalLine(x, static_cast<float>(graph.getY()), static_cast<float>(graph.getBottom()));
    }
    g.setColour(juce::Colour(0xff65d6ff));
    g.drawHorizontalLine(graph.getCentreY(), static_cast<float>(graph.getX() + 10), static_cast<float>(graph.getRight() - 10));

    g.setColour(juce::Colour(0xff8e9aa6));
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    const auto labelY = graph.getBottom() + 12;
    g.drawText("HP", 24, labelY, 90, 18, juce::Justification::centred);
    for (int i = 0; i < 4; ++i)
        g.drawText("BAND " + juce::String(i + 1), 130 + i * 150, labelY, 130, 18, juce::Justification::centred);
    g.drawText("LP", getWidth() - 204, labelY, 90, 18, juce::Justification::centred);
    g.drawText("OUT", getWidth() - 108, labelY, 80, 18, juce::Justification::centred);
}

void RefEQAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(juce::jmax(260, getHeight() / 3 + 88));
    const auto total = area.getWidth();
    const auto side = juce::jmax(88, total / 9);

    highPass_.setBounds(area.removeFromLeft(side).reduced(4));
    for (auto& band : bands_) {
        auto column = area.removeFromLeft(juce::jmax(135, area.getWidth() / static_cast<int>(bands_.size() + 2)));
        auto top = column.removeFromTop(column.getHeight() / 2);
        band.frequency.setBounds(top.removeFromLeft(top.getWidth() / 2).reduced(3));
        band.gain.setBounds(top.reduced(3));
        band.q.setBounds(column.reduced(column.getWidth() / 4, 2));
    }
    lowPass_.setBounds(area.removeFromLeft(side).reduced(4));
    output_.setBounds(area.reduced(4));
}

} // namespace ehvota::refeq
