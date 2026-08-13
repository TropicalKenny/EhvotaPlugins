#include "PluginEditor.h"

namespace ehvota::refcomp {
namespace {

const auto canvas = juce::Colour(0xff0b0f14);
const auto panel = juce::Colour(0xff10161d);
const auto border = juce::Colour(0xff26313b);
const auto text = juce::Colour(0xffe6edf3);
const auto secondary = juce::Colour(0xff8e9aa6);
const auto cyan = juce::Colour(0xff65d6ff);
const auto amber = juce::Colour(0xffe8a85b);

void configureCombo(juce::ComboBox& combo)
{
    combo.setColour(juce::ComboBox::backgroundColourId, panel);
    combo.setColour(juce::ComboBox::outlineColourId, border);
    combo.setColour(juce::ComboBox::textColourId, text);
    combo.setColour(juce::ComboBox::arrowColourId, secondary);
}

} // namespace

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

    mode_.addItem("Stereo Linked", 1);
    mode_.addItem("Dual Mono", 2);
    mode_.addItem("Mid / Side", 3);
    configureCombo(mode_);
    addAndMakeVisible(mode_);

    for (const auto& item : { juce::String("1/256"), juce::String("1/128"), juce::String("1/64"), juce::String("1/32"), juce::String("1/16") })
        attackNote_.addItem(item, attackNote_.getNumItems() + 1);
    for (const auto& item : { juce::String("1/16"), juce::String("1/8"), juce::String("1/4"), juce::String("1/2"), juce::String("1/1") })
        releaseNote_.addItem(item, releaseNote_.getNumItems() + 1);
    for (auto* combo : { &attackModifier_, &releaseModifier_ }) {
        combo->addItem("Straight", 1);
        combo->addItem("Dotted", 2);
        combo->addItem("Triplet", 3);
    }
    for (auto* combo : { &attackNote_, &attackModifier_, &releaseNote_, &releaseModifier_ }) {
        configureCombo(*combo);
        addAndMakeVisible(*combo);
    }

    for (auto* button : { &attackSync_, &releaseSync_ }) {
        button->setColour(juce::ToggleButton::textColourId, secondary);
        button->setColour(juce::ToggleButton::tickColourId, cyan);
        button->setColour(juce::ToggleButton::tickDisabledColourId, border);
        addAndMakeVisible(*button);
    }

    auto& state = processor_.parameters();
    modeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, "mode", mode_);
    attackSyncAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "attackSync", attackSync_);
    attackNoteAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, "attackNote", attackNote_);
    attackModifierAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, "attackModifier", attackModifier_);
    releaseSyncAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(state, "releaseSync", releaseSync_);
    releaseNoteAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, "releaseNote", releaseNote_);
    releaseModifierAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(state, "releaseModifier", releaseModifier_);

    gainReductionLabel_.setJustificationType(juce::Justification::centred);
    gainReductionLabel_.setColour(juce::Label::textColourId, amber);
    gainReductionLabel_.setFont(juce::FontOptions(24.0f).withStyle("Bold"));
    addAndMakeVisible(gainReductionLabel_);

    timingLabel_.setJustificationType(juce::Justification::centred);
    timingLabel_.setColour(juce::Label::textColourId, secondary);
    timingLabel_.setFont(juce::FontOptions(10.5f));
    addAndMakeVisible(timingLabel_);

    setResizable(true, true);
    setResizeLimits(760, 520, 1400, 900);
    setSize(980, 680);
    startTimerHz(20);
}

void RefCompAudioProcessorEditor::configureKnob(juce::Slider& slider, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 84, 20);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::rotarySliderFillColourId, cyan);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, border);
    slider.setColour(juce::Slider::textBoxTextColourId, text);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, canvas);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void RefCompAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(canvas);
    g.setColour(text);
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawText("Ehvota RefComp", 20, 14, getWidth() - 40, 30, juce::Justification::centredLeft);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("REFERENCE-LINKED DYNAMICS · LINKED / DUAL MONO / MID-SIDE · OPTIONAL HOST-SYNC TIMING", 20, 44, getWidth() - 40, 18, juce::Justification::centredLeft);

    auto meter = juce::Rectangle<int>(20, 92, getWidth() - 40, 138);
    g.setColour(panel);
    g.fillRoundedRectangle(meter.toFloat(), 8.0f);
    g.setColour(border);
    g.drawRoundedRectangle(meter.toFloat().reduced(0.5f), 8.0f, 1.0f);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("GAIN REDUCTION", meter.getX() + 18, meter.getY() + 14, meter.getWidth() - 36, 20, juce::Justification::centred);

    auto syncPanel = juce::Rectangle<int>(20, 244, getWidth() - 40, 82);
    g.setColour(panel);
    g.fillRoundedRectangle(syncPanel.toFloat(), 8.0f);
    g.setColour(border);
    g.drawRoundedRectangle(syncPanel.toFloat().reduced(0.5f), 8.0f, 1.0f);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText("COMPRESSION TIMING", syncPanel.getX() + 14, syncPanel.getY() + 8, 180, 18, juce::Justification::centredLeft);

    const std::array<const char*, 7> labels { "THRESHOLD", "RATIO", "ATTACK", "RELEASE", "KNEE", "MAKEUP", "MIX" };
    const auto controlsTop = 372;
    const auto width = (getWidth() - 40) / static_cast<int>(labels.size());
    for (std::size_t i = 0; i < labels.size(); ++i) {
        g.setColour(secondary);
        g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
        g.drawText(labels[i], 20 + static_cast<int>(i) * width, controlsTop - 22, width, 18, juce::Justification::centred);
    }

    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f));
    g.drawFittedText("Reference-driven changes remain bounded proposals. Mid/Side mode compresses encoded M and S independently, then reconstructs stereo; it does not alter width unless the gain reduction differs.", 26, getHeight() - 62, getWidth() - 52, 38, juce::Justification::centred, 2);
}

void RefCompAudioProcessorEditor::resized()
{
    mode_.setBounds(getWidth() - 210, 18, 190, 34);
    gainReductionLabel_.setBounds(80, 130, getWidth() - 160, 52);
    timingLabel_.setBounds(80, 184, getWidth() - 160, 24);

    auto timing = juce::Rectangle<int>(34, 272, getWidth() - 68, 40);
    const auto half = timing.getWidth() / 2;
    auto attackArea = timing.removeFromLeft(half).reduced(4, 0);
    attackSync_.setBounds(attackArea.removeFromLeft(112));
    attackNote_.setBounds(attackArea.removeFromLeft(92).reduced(3));
    attackModifier_.setBounds(attackArea.removeFromLeft(108).reduced(3));

    auto releaseArea = timing.reduced(4, 0);
    releaseSync_.setBounds(releaseArea.removeFromLeft(118));
    releaseNote_.setBounds(releaseArea.removeFromLeft(92).reduced(3));
    releaseModifier_.setBounds(releaseArea.removeFromLeft(108).reduced(3));

    auto controlsArea = juce::Rectangle<int>(20, 372, getWidth() - 40, juce::jmax(120, getHeight() - 462));
    const auto width = controlsArea.getWidth() / static_cast<int>(controls_.size());
    for (auto& control : controls_)
        control.setBounds(controlsArea.removeFromLeft(width).reduced(5));
}

void RefCompAudioProcessorEditor::timerCallback()
{
    gainReductionLabel_.setText(
        "−" + juce::String(processor_.gainReductionDb(), 2) + " dB",
        juce::dontSendNotification);
    timingLabel_.setText(
        "Active attack " + juce::String(processor_.activeAttackMs(), 2) + " ms  ·  release "
            + juce::String(processor_.activeReleaseMs(), 2) + " ms",
        juce::dontSendNotification);

    const auto attackSynced = attackSync_.getToggleState();
    const auto releaseSynced = releaseSync_.getToggleState();
    controls_[2].setEnabled(!attackSynced);
    attackNote_.setEnabled(attackSynced);
    attackModifier_.setEnabled(attackSynced);
    controls_[3].setEnabled(!releaseSynced);
    releaseNote_.setEnabled(releaseSynced);
    releaseModifier_.setEnabled(releaseSynced);
}

} // namespace ehvota::refcomp
