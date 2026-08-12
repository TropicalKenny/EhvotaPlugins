#include "ApprovedRefTimePreview.h"

#include "shared/ui/Theme.h"

namespace ehvota::previewapp {
namespace {

juce::Colour c(const juce::uint32 value) { return juce::Colour(value); }

void panel(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(c(ehvota::ui::colour::base));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);
    g.setColour(c(ehvota::ui::colour::borderSubtle));
    g.drawRoundedRectangle(area.toFloat().reduced(0.5f), 8.0f, 1.0f);
}

} // namespace

ApprovedRefTimePreview::ApprovedRefTimePreview()
{
    divisionSelector_.addItem("1/16", 1);
    divisionSelector_.addItem("1/32", 2);
    divisionSelector_.addItem("1/64", 3);
    divisionSelector_.setSelectedId(2, juce::dontSendNotification);
    divisionSelector_.setTooltip("Choose the musical division.");
    addAndMakeVisible(divisionSelector_);

    modifierSelector_.addItem("Straight", 1);
    modifierSelector_.addItem("Dotted", 2);
    modifierSelector_.addItem("Triplet", 3);
    modifierSelector_.setSelectedId(2, juce::dontSendNotification);
    modifierSelector_.setTooltip("Choose straight, dotted, or triplet timing.");
    addAndMakeVisible(modifierSelector_);

    outputModeSelector_.addItem("DELAYED ONLY", 1);
    outputModeSelector_.addItem("DRY + DELAYED", 2);
    outputModeSelector_.addItem("DRY ONLY", 3);
    outputModeSelector_.setSelectedId(1, juce::dontSendNotification);
    outputModeSelector_.setTooltip("Delayed Only is the default external reverb pre-delay mode.");
    addAndMakeVisible(outputModeSelector_);

    for (auto* button : { &copyExactButton_, &copyRoundedButton_, &copyNumberButton_, &copySamplesButton_ }) {
        addAndMakeVisible(*button);
        button->setWantsKeyboardFocus(true);
    }

    copyExactButton_.setTooltip("Copy the exact millisecond value with unit.");
    copyRoundedButton_.setTooltip("Copy the nearest whole-millisecond value.");
    copyNumberButton_.setTooltip("Copy only the exact numeric millisecond value.");
    copySamplesButton_.setTooltip("Copy the exact sample count for the current sample rate.");
}

void ApprovedRefTimePreview::paint(juce::Graphics& g)
{
    g.fillAll(c(ui::colour::canvas));
    auto bounds = getLocalBounds().reduced(14);

    auto header = bounds.removeFromTop(60);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(22.0f, true));
    g.drawText("Ehvota", header.removeFromLeft(92), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::liveMix));
    g.setFont(ui::font(16.0f, true));
    g.drawText("RefTime", header.removeFromLeft(78), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::positive));
    g.fillEllipse(static_cast<float>(header.getX()), static_cast<float>(header.getCentreY() - 3), 6.0f, 6.0f);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(11.0f, true));
    g.drawText("  HOST SYNC · Ableton Live 12 Suite", header.removeFromLeft(240), juce::Justification::centredLeft);
    g.drawText("120.000 BPM    ·    4 / 4    ·    48 kHz", header, juce::Justification::centredRight);

    auto footer = bounds.removeFromBottom(76);
    bounds.removeFromBottom(10);

    auto left = bounds.removeFromLeft(220);
    bounds.removeFromLeft(10);
    auto centre = bounds.removeFromLeft(juce::jmax(300, bounds.getWidth() - 250));
    bounds.removeFromLeft(10);
    auto right = bounds;

    panel(g, left);
    panel(g, centre);
    panel(g, right);

    auto l = left.reduced(16);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.0f, true));
    g.drawText("TIMING SOURCE", l.removeFromTop(22), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::positive));
    g.setFont(ui::font(13.0f, true));
    g.drawText("HOST", l.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::numericFont(11.0f));
    g.drawText("Tempo      120.000 BPM", l.removeFromTop(25), juce::Justification::centredLeft);
    g.drawText("Meter      4 / 4", l.removeFromTop(25), juce::Justification::centredLeft);
    g.drawText("Sample rate 48,000 Hz", l.removeFromTop(25), juce::Justification::centredLeft);
    l.removeFromTop(12);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.0f, true));
    g.drawText("TECHNICAL LATENCY", l.removeFromTop(22), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::positive));
    g.setFont(ui::numericFont(14.0f, true));
    g.drawText("0 samples", l.removeFromTop(26), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textMuted));
    g.setFont(ui::font(10.5f));
    g.drawFittedText("Intentional musical delay is not reported to Ableton as plugin latency.", l, juce::Justification::topLeft, 3);

    auto cc = centre.reduced(18);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.0f, true));
    g.drawText("EXACT DELAY", cc.removeFromTop(24), juce::Justification::centred);
    g.setColour(c(ui::colour::liveMix));
    g.setFont(ui::numericFont(46.0f, true));
    g.drawText("93.750", cc.removeFromTop(68), juce::Justification::centred);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(18.0f, true));
    g.drawText("ms", cc.removeFromTop(28), juce::Justification::centred);

    auto readouts = cc.removeFromTop(52);
    const auto third = readouts.getWidth() / 3;
    auto rounded = readouts.removeFromLeft(third);
    auto samples = readouts.removeFromLeft(third);
    auto frequency = readouts;
    g.setColour(c(ui::colour::textMuted));
    g.setFont(ui::font(9.5f, true));
    g.drawText("ROUNDED", rounded.removeFromTop(18), juce::Justification::centred);
    g.drawText("SAMPLES", samples.removeFromTop(18), juce::Justification::centred);
    g.drawText("FREQUENCY", frequency.removeFromTop(18), juce::Justification::centred);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::numericFont(14.0f, true));
    g.drawText("94 ms", rounded, juce::Justification::centred);
    g.drawText("4,500", samples, juce::Justification::centred);
    g.drawText("10.667 Hz", frequency, juce::Justification::centred);

    cc.removeFromTop(10);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.5f));
    g.drawFittedText("Source  →  RefTime  →  Third-party reverb / delay\nFor external reverb pre-delay: set the following effect 100% wet and its own pre-delay to 0 ms.", cc, juce::Justification::centred, 3);

    auto r = right.reduced(16);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.0f, true));
    g.drawText("TEMPO CHANGE", r.removeFromTop(22), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(12.0f, true));
    g.drawText("Clean Crossfade", r.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textMuted));
    g.setFont(ui::font(10.5f));
    g.drawFittedText("Alternative modes: Instant and creative Tape Glide. Host tempo changes must not click.", r.removeFromTop(70), juce::Justification::topLeft, 4);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.0f, true));
    g.drawText("USE CASE", r.removeFromTop(22), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(11.0f));
    g.drawFittedText("Place RefTime before Valhalla or another third-party effect on a send. RefTime controls effect onset; it does not remotely edit the third-party plugin.", r, juce::Justification::topLeft, 6);

    panel(g, footer);
    auto f = footer.reduced(18, 12);
    g.setColour(c(ui::colour::proposed));
    g.setFont(ui::font(10.0f, true));
    g.drawText("OUTPUT", f.removeFromLeft(70), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.5f));
    g.drawText("Delayed Only is the default external pre-delay path. Musical delay remains intentional audio timing, not host compensation.", f, juce::Justification::centredLeft);
}

void ApprovedRefTimePreview::resized()
{
    auto bounds = getLocalBounds().reduced(14);
    bounds.removeFromTop(60);
    auto footer = bounds.removeFromBottom(76);
    bounds.removeFromBottom(10);

    auto left = bounds.removeFromLeft(220);
    bounds.removeFromLeft(10);
    auto centre = bounds.removeFromLeft(juce::jmax(300, bounds.getWidth() - 250));
    bounds.removeFromLeft(10);
    auto right = bounds;

    auto l = left.reduced(16);
    l.removeFromTop(172);
    divisionSelector_.setBounds(l.removeFromTop(32));
    l.removeFromTop(8);
    modifierSelector_.setBounds(l.removeFromTop(32));

    auto r = right.reduced(16);
    r.removeFromTop(200);
    outputModeSelector_.setBounds(r.removeFromTop(32));

    auto f = footer.reduced(18, 18);
    copyExactButton_.setBounds(f.removeFromRight(132));
    f.removeFromRight(8);
    copyRoundedButton_.setBounds(f.removeFromRight(122));
    f.removeFromRight(8);
    copyNumberButton_.setBounds(f.removeFromRight(116));
    f.removeFromRight(8);
    copySamplesButton_.setBounds(f.removeFromRight(116));

    juce::ignoreUnused(centre);
}

} // namespace ehvota::previewapp
