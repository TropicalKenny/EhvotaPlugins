#include "PluginEditor.h"

#include <array>
#include <cmath>

namespace ehvota::refhub {
namespace {

const auto canvas = juce::Colour(0xff090d12);
const auto panel = juce::Colour(0xff10161d);
const auto border = juce::Colour(0xff26313b);
const auto text = juce::Colour(0xffe8edf2);
const auto secondary = juce::Colour(0xff8e9aa6);
const auto cyan = juce::Colour(0xff62d8ff);
const auto violet = juce::Colour(0xff9278ff);
const auto amber = juce::Colour(0xffe8a85b);
const auto green = juce::Colour(0xff64d98b);

void drawPanel(juce::Graphics& g, const juce::Rectangle<int> area)
{
    g.setColour(panel);
    g.fillRoundedRectangle(area.toFloat(), 8.0f);
    g.setColour(border);
    g.drawRoundedRectangle(area.toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void drawWave(juce::Graphics& g, const juce::Rectangle<float> area, const juce::Colour colour, const float phase)
{
    juce::Path path;
    for (int x = 0; x <= static_cast<int>(area.getWidth()); ++x) {
        const auto t = static_cast<float>(x) / juce::jmax(1.0f, area.getWidth());
        const auto y = area.getCentreY()
            - (std::sin(t * 90.0f + phase) + 0.4f * std::sin(t * 237.0f + phase * 0.5f))
                * area.getHeight() * 0.20f;
        const auto px = area.getX() + static_cast<float>(x);
        if (x == 0) path.startNewSubPath(px, y); else path.lineTo(px, y);
    }
    g.setColour(colour.withAlpha(0.9f));
    g.strokePath(path, juce::PathStrokeType(1.1f));
}

} // namespace

RefHubAudioProcessorEditor::RefHubAudioProcessorEditor(RefHubAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processor_(processor)
{
    referenceSelector_.addItem("Ayo Nights", 1);
    referenceSelector_.setSelectedId(1, juce::dontSendNotification);
    sectionSelector_.addItem("Chorus 1", 1);
    sectionSelector_.addItem("Verse 1", 2);
    sectionSelector_.setSelectedId(1, juce::dontSendNotification);
    levelMatchSelector_.addItem("AUTO / SECTION", 1);
    levelMatchSelector_.addItem("WHOLE TRACK", 2);
    levelMatchSelector_.addItem("OFF", 3);
    levelMatchSelector_.setSelectedId(1, juce::dontSendNotification);

    for (auto* component : { static_cast<juce::Component*>(&referenceSelector_), static_cast<juce::Component*>(&sectionSelector_), static_cast<juce::Component*>(&levelMatchSelector_), static_cast<juce::Component*>(&mixButton_), static_cast<juce::Component*>(&referenceButton_), static_cast<juce::Component*>(&blindButton_), static_cast<juce::Component*>(&captureButton_), static_cast<juce::Component*>(&proposalButton_), static_cast<juce::Component*>(&liveMeterLabel_) })
        addAndMakeVisible(*component);

    mixButton_.setClickingTogglesState(true);
    referenceButton_.setClickingTogglesState(true);
    mixButton_.setToggleState(true, juce::dontSendNotification);
    mixButton_.onClick = [this] {
        mixButton_.setToggleState(true, juce::dontSendNotification);
        referenceButton_.setToggleState(false, juce::dontSendNotification);
    };
    referenceButton_.onClick = [this] {
        referenceButton_.setToggleState(true, juce::dontSendNotification);
        mixButton_.setToggleState(false, juce::dontSendNotification);
    };

    proposalButton_.setTooltip("Creates a reviewable proposal only. No processing is applied automatically.");
    liveMeterLabel_.setColour(juce::Label::textColourId, secondary);
    liveMeterLabel_.setJustificationType(juce::Justification::centredRight);

    setResizable(true, true);
    setResizeLimits(860, 600, 1800, 1200);
    setSize(1180, 760);
    startTimerHz(15);
}

void RefHubAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(canvas);
    auto bounds = getLocalBounds().reduced(12);
    auto header = bounds.removeFromTop(62);

    g.setColour(text);
    g.setFont(juce::FontOptions(22.0f).withStyle("Bold"));
    g.drawText("Ehvota", header.removeFromLeft(92), juce::Justification::centredLeft);
    g.setColour(cyan);
    g.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
    g.drawText("RefHub", header.removeFromLeft(70), juce::Justification::centredLeft);
    g.setColour(green);
    g.fillEllipse(168.0f, 44.0f, 6.0f, 6.0f);
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.drawText("LIVE CONNECTED", 180, 36, 120, 20, juce::Justification::centredLeft);

    auto rail = bounds.removeFromLeft(82);
    drawPanel(g, rail);
    const std::array<const char*, 4> nav { "COMPARE", "SOURCES", "PROCESS", "HISTORY" };
    auto navY = rail.getY() + 22;
    for (std::size_t i = 0; i < nav.size(); ++i) {
        auto row = juce::Rectangle<int>(rail.getX() + 6, navY, rail.getWidth() - 12, 68);
        if (i == 0) {
            g.setColour(cyan.withAlpha(0.1f));
            g.fillRoundedRectangle(row.toFloat(), 7.0f);
            g.setColour(cyan);
            g.fillRect(row.getX(), row.getY(), 3, row.getHeight());
        }
        g.setColour(i == 0 ? cyan : secondary);
        g.setFont(juce::FontOptions(10.0f).withStyle(i == 0 ? "Bold" : "Regular"));
        g.drawText(nav[i], row, juce::Justification::centred);
        navY += 82;
    }

    bounds.removeFromLeft(10);
    auto footer = bounds.removeFromBottom(24);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("Ableton Connected · 89.00 BPM · 4 / 4 · 44.1 kHz", footer, juce::Justification::centredLeft);

    bounds.removeFromBottom(8);
    auto timeline = bounds.removeFromTop(168);
    drawPanel(g, timeline);
    auto tl = timeline.reduced(16, 10);
    const std::array<const char*, 6> sections { "Intro", "Verse 1", "Chorus 1", "Verse 2", "Chorus 2", "Outro" };
    auto labels = tl.removeFromTop(24);
    const auto sectionWidth = labels.getWidth() / static_cast<int>(sections.size());
    for (std::size_t i = 0; i < sections.size(); ++i) {
        auto cell = labels.removeFromLeft(sectionWidth);
        if (i == 2) {
            g.setColour(violet.withAlpha(0.12f));
            g.fillRoundedRectangle(cell.toFloat(), 4.0f);
        }
        g.setColour(i == 2 ? violet : secondary);
        g.setFont(juce::FontOptions(10.0f));
        g.drawText(sections[i], cell, juce::Justification::centred);
    }

    auto mixWave = tl.removeFromTop(42).toFloat().withTrimmedLeft(72.0f);
    auto refWave = tl.removeFromTop(42).toFloat().withTrimmedLeft(72.0f);
    g.setColour(cyan);
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText("YOUR MIX", timeline.getX() + 12, static_cast<int>(mixWave.getY()), 66, 32, juce::Justification::centredLeft);
    g.setColour(violet);
    g.drawText("REFERENCE", timeline.getX() + 12, static_cast<int>(refWave.getY()), 66, 32, juce::Justification::centredLeft);
    drawWave(g, mixWave, cyan, 0.1f);
    drawWave(g, refWave, violet, 0.8f);

    bounds.removeFromTop(10);
    auto insight = bounds.removeFromBottom(126);
    bounds.removeFromBottom(10);
    drawPanel(g, insight);
    drawPanel(g, bounds);

    auto analyser = bounds.reduced(18, 12);
    auto tabs = analyser.removeFromTop(34);
    const std::array<const char*, 4> lenses { "TONE", "DYNAMICS", "SPACE", "COMPONENTS" };
    for (std::size_t i = 0; i < lenses.size(); ++i) {
        auto tab = tabs.removeFromLeft(118);
        g.setColour(i == 0 ? cyan : secondary);
        g.setFont(juce::FontOptions(10.5f).withStyle(i == 0 ? "Bold" : "Regular"));
        g.drawText(lenses[i], tab, juce::Justification::centred);
    }

    auto graph = analyser.reduced(0, 6).toFloat();
    g.setColour(border.withAlpha(0.7f));
    for (int i = 1; i < 10; ++i) {
        const auto x = graph.getX() + graph.getWidth() * static_cast<float>(i) / 10.0f;
        g.drawVerticalLine(static_cast<int>(x), graph.getY(), graph.getBottom());
    }
    for (int i = 1; i < 6; ++i) {
        const auto y = graph.getY() + graph.getHeight() * static_cast<float>(i) / 6.0f;
        g.drawHorizontalLine(static_cast<int>(y), graph.getX(), graph.getRight());
    }

    const auto selectedStart = graph.getX() + graph.getWidth() * 0.30f;
    const auto selectedEnd = graph.getX() + graph.getWidth() * 0.46f;
    g.setColour(amber.withAlpha(0.10f));
    g.fillRect(juce::Rectangle<float>(selectedStart, graph.getY(), selectedEnd - selectedStart, graph.getHeight()));

    juce::Path mixPath;
    juce::Path refPath;
    for (int x = 0; x <= static_cast<int>(graph.getWidth()); ++x) {
        const auto t = static_cast<float>(x) / juce::jmax(1.0f, graph.getWidth());
        const auto mixY = graph.getCentreY() - (0.48f * std::sin(t * 7.0f) + 0.18f * std::sin(t * 29.0f) - 0.35f * t) * graph.getHeight() * 0.35f;
        const auto refY = graph.getCentreY() - (0.42f * std::sin(t * 7.0f + 0.7f) + 0.15f * std::sin(t * 25.0f) - 0.39f * t) * graph.getHeight() * 0.35f;
        const auto px = graph.getX() + static_cast<float>(x);
        if (x == 0) { mixPath.startNewSubPath(px, mixY); refPath.startNewSubPath(px, refY); }
        else { mixPath.lineTo(px, mixY); refPath.lineTo(px, refY); }
    }
    g.setColour(cyan);
    g.strokePath(mixPath, juce::PathStrokeType(1.5f));
    g.setColour(violet);
    g.strokePath(refPath, juce::PathStrokeType(1.5f));

    auto callout = juce::Rectangle<int>(static_cast<int>(selectedStart + 24), bounds.getY() + 72, 200, 112);
    g.setColour(juce::Colour(0xee0b1016));
    g.fillRoundedRectangle(callout.toFloat(), 8.0f);
    g.setColour(amber);
    g.drawRoundedRectangle(callout.toFloat().reduced(0.5f), 8.0f, 1.0f);
    auto co = callout.reduced(12, 8);
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText("LOW-MID BUMP", co.removeFromTop(18), juce::Justification::centredLeft);
    g.setColour(text);
    g.setFont(juce::FontOptions(18.0f).withStyle("Bold"));
    g.drawText("220 – 410 Hz", co.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(amber);
    g.drawText("+1.8 dB", co.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("Confidence: High", co, juce::Justification::centredLeft);

    auto ins = insight.reduced(18, 12);
    g.setColour(amber);
    g.setFont(juce::FontOptions(9.5f).withStyle("Bold"));
    g.drawText("EHVOTA INSIGHT", ins.removeFromTop(18), juce::Justification::centredLeft);
    g.setColour(text);
    g.setFont(juce::FontOptions(18.0f).withStyle("Bold"));
    g.drawText("Low-mid buildup · mainly Drums + Bass", ins.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(secondary);
    g.setFont(juce::FontOptions(10.5f));
    g.drawFittedText("Listen first, compare level-matched, then create a bounded proposal. Nothing changes automatically.", ins, juce::Justification::topLeft, 3);
}

void RefHubAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(12);
    auto header = bounds.removeFromTop(62);
    header.removeFromLeft(310);
    referenceSelector_.setBounds(header.removeFromLeft(160).reduced(3, 10));
    sectionSelector_.setBounds(header.removeFromLeft(145).reduced(3, 10));
    mixButton_.setBounds(header.removeFromLeft(62).reduced(2, 10));
    referenceButton_.setBounds(header.removeFromLeft(62).reduced(2, 10));
    blindButton_.setBounds(header.removeFromLeft(72).reduced(2, 10));
    captureButton_.setBounds(header.removeFromLeft(86).reduced(2, 10));
    levelMatchSelector_.setBounds(header.removeFromLeft(138).reduced(2, 10));
    liveMeterLabel_.setBounds(header.reduced(4, 10));

    bounds.removeFromLeft(92);
    bounds.removeFromBottom(32);
    bounds.removeFromTop(168 + 10);
    auto insight = bounds.removeFromBottom(126);
    auto actions = insight.reduced(18, 42);
    proposalButton_.setBounds(actions.removeFromRight(160).reduced(2));
}

void RefHubAudioProcessorEditor::timerCallback()
{
    liveMeterLabel_.setText(
        "Peak " + juce::String(processor_.livePeakDb(), 1) + " dBFS · RMS " + juce::String(processor_.liveRmsDb(), 1) + " dBFS",
        juce::dontSendNotification);
}

} // namespace ehvota::refhub
