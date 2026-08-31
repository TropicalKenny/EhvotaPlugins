#include "PluginEditor.h"

namespace ehvota::reftime {

RefTimeAudioProcessorEditor::RefTimeAudioProcessorEditor(RefTimeAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processor_(processor)
{
    divisionSelector_.addItemList({ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64" }, 1);
    modifierSelector_.addItemList({ "Straight", "Dotted", "Triplet" }, 1);
    outputModeSelector_.addItemList({ "Delayed Only", "Dry + Delayed", "Dry Only" }, 1);

    for (auto* component : { static_cast<juce::Component*>(&divisionSelector_), static_cast<juce::Component*>(&modifierSelector_), static_cast<juce::Component*>(&outputModeSelector_), static_cast<juce::Component*>(&exactLabel_), static_cast<juce::Component*>(&hostLabel_), static_cast<juce::Component*>(&detailLabel_), static_cast<juce::Component*>(&copyExactButton_), static_cast<juce::Component*>(&copyNumberButton_), static_cast<juce::Component*>(&copySamplesButton_) })
        addAndMakeVisible(*component);

    exactLabel_.setJustificationType(juce::Justification::centred);
    exactLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff65d6ff));
    exactLabel_.setFont(juce::FontOptions(52.0f).withStyle("Bold"));
    hostLabel_.setJustificationType(juce::Justification::centred);
    hostLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff9ba7b4));
    detailLabel_.setJustificationType(juce::Justification::centred);
    detailLabel_.setColour(juce::Label::textColourId, juce::Colour(0xffd8dee6));

    divisionAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor_.parameters(), "division", divisionSelector_);
    modifierAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor_.parameters(), "modifier", modifierSelector_);
    outputModeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor_.parameters(), "outputMode", outputModeSelector_);

    copyExactButton_.onClick = [this] {
        copyText(juce::String(processor_.exactMilliseconds(), 3) + " ms");
    };
    copyNumberButton_.onClick = [this] {
        copyText(juce::String(processor_.exactMilliseconds(), 3));
    };
    copySamplesButton_.onClick = [this] {
        copyText(juce::String(processor_.exactSamples()));
    };

    setResizable(true, true);
    setResizeLimits(620, 420, 1000, 700);
    setSize(720, 500);
    startTimerHz(15);
}

void RefTimeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0b0f14));
    g.setColour(juce::Colour(0xffe6edf3));
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawText("Ehvota RefTime", 20, 14, getWidth() - 40, 30, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff27313b));
    auto body = getLocalBounds().reduced(18).withTrimmedTop(52);
    g.fillRoundedRectangle(body.toFloat(), 10.0f);
    g.setColour(juce::Colour(0xff384550));
    g.drawRoundedRectangle(body.toFloat().reduced(0.5f), 10.0f, 1.0f);

    g.setColour(juce::Colour(0xff7f8b98));
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("NOTE DIVISION", 34, 86, 150, 20, juce::Justification::centredLeft);
    g.drawText("MODIFIER", 34, 156, 150, 20, juce::Justification::centredLeft);
    g.drawText("OUTPUT MODE", getWidth() - 212, 86, 150, 20, juce::Justification::centredLeft);
    g.drawText("EXACT DELAY", 220, 86, getWidth() - 440, 20, juce::Justification::centred);

    g.setColour(juce::Colour(0xff65d6ff));
    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.drawText("Source  →  RefTime  →  Third-party effect", 200, getHeight() - 142, getWidth() - 400, 24, juce::Justification::centred);

    g.setColour(juce::Colour(0xff7f8b98));
    g.setFont(juce::FontOptions(10.5f));
    g.drawFittedText("Delayed Only is the default send workflow. RefTime creates intentional musical delay and reports 0 samples of technical plugin latency.", 190, getHeight() - 114, getWidth() - 380, 42, juce::Justification::centred, 2);
}

void RefTimeAudioProcessorEditor::resized()
{
    divisionSelector_.setBounds(34, 110, 150, 34);
    modifierSelector_.setBounds(34, 180, 150, 34);
    outputModeSelector_.setBounds(getWidth() - 212, 110, 178, 34);

    exactLabel_.setBounds(206, 108, getWidth() - 412, 82);
    hostLabel_.setBounds(200, 198, getWidth() - 400, 26);
    detailLabel_.setBounds(190, 228, getWidth() - 380, 26);

    auto actions = juce::Rectangle<int>(34, getHeight() - 58, getWidth() - 68, 34);
    const auto buttonWidth = juce::jmin(150, actions.getWidth() / 3 - 8);
    copyExactButton_.setBounds(actions.removeFromLeft(buttonWidth));
    actions.removeFromLeft(8);
    copyNumberButton_.setBounds(actions.removeFromLeft(buttonWidth));
    actions.removeFromLeft(8);
    copySamplesButton_.setBounds(actions.removeFromLeft(buttonWidth));
}

void RefTimeAudioProcessorEditor::timerCallback()
{
    exactLabel_.setText(juce::String(processor_.exactMilliseconds(), 3) + " ms", juce::dontSendNotification);
    hostLabel_.setText("HOST SYNC · " + juce::String(processor_.hostBpm(), 3) + " BPM", juce::dontSendNotification);
    detailLabel_.setText("Exact samples: " + juce::String(processor_.exactSamples()), juce::dontSendNotification);
}

void RefTimeAudioProcessorEditor::copyText(const juce::String& text)
{
    juce::SystemClipboard::copyTextToClipboard(text);
}

} // namespace ehvota::reftime
