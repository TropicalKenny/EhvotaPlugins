#include "PluginEditor.h"

namespace ehvota::reftap {

RefTapAudioProcessorEditor::RefTapAudioProcessorEditor(RefTapAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processor_(processor)
{
    for (auto* label : { &peakLabel_, &rmsLabel_, &correlationLabel_ }) {
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colour(0xffe6edf3));
        label->setFont(juce::FontOptions(16.0f));
        addAndMakeVisible(*label);
    }

    setResizable(true, true);
    setResizeLimits(320, 180, 720, 420);
    setSize(440, 240);
    startTimerHz(20);
}

void RefTapAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0d1117));

    g.setColour(juce::Colour(0xffe6edf3));
    g.setFont(juce::FontOptions(20.0f).withStyle("Bold"));
    g.drawText("Ehvota RefTap", 18, 12, getWidth() - 36, 28, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff65d6ff));
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("BIT-TRANSPARENT SOURCE METER", 18, 40, getWidth() - 36, 20, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff25303a));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(14.0f).withTrimmedTop(54.0f), 8.0f, 1.0f);
}

void RefTapAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(62);
    const auto width = area.getWidth() / 3;
    peakLabel_.setBounds(area.removeFromLeft(width).reduced(4));
    rmsLabel_.setBounds(area.removeFromLeft(width).reduced(4));
    correlationLabel_.setBounds(area.reduced(4));
}

void RefTapAudioProcessorEditor::timerCallback()
{
    peakLabel_.setText("PEAK\n" + juce::String(processor_.peakDb(), 1) + " dBFS", juce::dontSendNotification);
    rmsLabel_.setText("RMS\n" + juce::String(processor_.rmsDb(), 1) + " dBFS", juce::dontSendNotification);
    correlationLabel_.setText("CORRELATION\n" + juce::String(processor_.correlation(), 2), juce::dontSendNotification);
}

} // namespace ehvota::reftap
