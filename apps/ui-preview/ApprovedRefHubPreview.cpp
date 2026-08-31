#include "ApprovedRefHubPreview.h"

#include "shared/ui/Theme.h"

#include <array>
#include <cmath>

namespace ehvota::previewapp {
namespace {

constexpr int pad = 12;
constexpr int railWidth = 82;
constexpr int headerHeight = 62;
constexpr int timelineHeight = 178;
constexpr int insightHeight = 132;

juce::Colour c(const juce::uint32 value)
{
    return juce::Colour(value);
}

void drawRoundPanel(juce::Graphics& g, juce::Rectangle<int> area, const float radius = 8.0f)
{
    g.setColour(c(ehvota::ui::colour::base));
    g.fillRoundedRectangle(area.toFloat(), radius);
    g.setColour(c(ehvota::ui::colour::borderSubtle).withAlpha(0.85f));
    g.drawRoundedRectangle(area.toFloat().reduced(0.5f), radius, 1.0f);
}

void drawSectionWave(
    juce::Graphics& g,
    const juce::Rectangle<float> area,
    const juce::Colour colour,
    const float phase,
    const float gain)
{
    juce::Path p;
    const auto centre = area.getCentreY();
    const auto half = area.getHeight() * 0.45f;
    const auto width = juce::jmax(1.0f, area.getWidth());

    for (int x = 0; x <= static_cast<int>(width); ++x) {
        const auto t = static_cast<float>(x) / width;
        const auto envelope = 0.55f + 0.35f * std::sin((t + phase) * 9.0f);
        const auto texture = std::sin((t * 115.0f) + phase * 8.0f)
            + 0.48f * std::sin((t * 239.0f) + phase * 3.0f)
            + 0.22f * std::sin((t * 487.0f) + phase * 11.0f);
        const auto y = centre - texture * envelope * half * 0.30f * gain;
        const auto px = area.getX() + static_cast<float>(x);
        if (x == 0)
            p.startNewSubPath(px, y);
        else
            p.lineTo(px, y);
    }

    g.setColour(colour.withAlpha(0.92f));
    g.strokePath(p, juce::PathStrokeType(1.15f));
}

void drawSpectrum(
    juce::Graphics& g,
    const juce::Rectangle<float> area,
    const juce::Colour colour,
    const float offset,
    const float phase,
    const float alpha)
{
    juce::Path line;
    juce::Path fill;
    const auto width = juce::jmax(1.0f, area.getWidth());

    for (int x = 0; x <= static_cast<int>(width); ++x) {
        const auto t = static_cast<float>(x) / width;
        const auto broad = 0.48f * std::sin((t * 6.6f) + phase)
            + 0.24f * std::sin((t * 20.0f) + phase * 0.7f)
            + 0.12f * std::sin((t * 49.0f) + phase * 1.7f);
        const auto tilt = -0.55f * t;
        const auto normalized = juce::jlimit(-1.0f, 1.0f, broad + tilt + offset);
        const auto y = area.getCentreY() - normalized * area.getHeight() * 0.38f;
        const auto px = area.getX() + static_cast<float>(x);
        if (x == 0) {
            line.startNewSubPath(px, y);
            fill.startNewSubPath(px, area.getBottom());
            fill.lineTo(px, y);
        }
        else {
            line.lineTo(px, y);
            fill.lineTo(px, y);
        }
    }

    fill.lineTo(area.getRight(), area.getBottom());
    fill.closeSubPath();
    g.setColour(colour.withAlpha(alpha));
    g.fillPath(fill);
    g.setColour(colour);
    g.strokePath(line, juce::PathStrokeType(1.5f));
}

} // namespace

ApprovedRefHubPreview::ApprovedRefHubPreview()
{
    setWantsKeyboardFocus(true);

    referenceSelector_.addItem("Ayo Nights", 1);
    referenceSelector_.addItem("River Glass", 2);
    referenceSelector_.setSelectedId(1, juce::dontSendNotification);
    referenceSelector_.setTooltip("Choose the reference package.");
    addAndMakeVisible(referenceSelector_);

    sectionSelector_.addItem("Chorus 1", 1);
    sectionSelector_.addItem("Verse 1", 2);
    sectionSelector_.addItem("Chorus 2", 3);
    sectionSelector_.setSelectedId(1, juce::dontSendNotification);
    sectionSelector_.setTooltip("Choose the paired section used for comparison.");
    addAndMakeVisible(sectionSelector_);

    configureButton(mixButton_, "Audition Your Mix.");
    configureButton(referenceButton_, "Audition the Reference. Reference audio never enters export.");
    configureButton(blindButton_, "Start a level-matched blind comparison.");
    configureButton(captureButton_, "Capture stable section evidence.");
    configureButton(inspectorButton_, "Show or hide the contextual inspector.");
    configureButton(listenRangeButton_, "Audition only the selected 220–410 Hz range with controlled audition gain.");
    configureButton(compareButton_, "Alternate Your Mix and Reference for the selected range.");
    configureButton(createProposalButton_, "Create a bounded Ehvota processing proposal. Nothing is applied automatically.");

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

    blindButton_.onClick = [this] {
        levelMatchState_.blindComparison = true;
        updateLevelMatchSummary();
    };

    inspectorButton_.onClick = [this] {
        inspectorOpen_ = !inspectorOpen_;
        resized();
        repaint();
    };

    levelMatchMode_.addItem("AUTO / SECTION", 1);
    levelMatchMode_.addItem("WHOLE TRACK", 2);
    levelMatchMode_.addItem("OFF", 3);
    levelMatchMode_.setSelectedId(1, juce::dontSendNotification);
    levelMatchMode_.setTooltip("Select the monitoring-only reference level match mode.");
    levelMatchMode_.onChange = [this] {
        switch (levelMatchMode_.getSelectedId()) {
        case 2:
            levelMatchState_.mode = preview::LevelMatchMode::wholeTrack;
            levelMatchState_.enabled = true;
            break;
        case 3:
            levelMatchState_.mode = preview::LevelMatchMode::off;
            levelMatchState_.enabled = false;
            break;
        default:
            levelMatchState_.mode = preview::LevelMatchMode::autoSection;
            levelMatchState_.enabled = true;
            break;
        }
        updateLevelMatchSummary();
    };
    addAndMakeVisible(levelMatchMode_);

    levelMatchSummary_.setFont(ui::numericFont(11.5f));
    levelMatchSummary_.setColour(juce::Label::textColourId, c(ui::colour::textSecondary));
    levelMatchSummary_.setJustificationType(juce::Justification::centredLeft);
    levelMatchSummary_.setMinimumHorizontalScale(0.8f);
    addAndMakeVisible(levelMatchSummary_);

    levelMatchState_.section = {
        -14.8,
        -10.9,
        -3.9,
        preview::EvidenceQuality::capturedStable,
        "Chorus 1 captured evidence"
    };
    levelMatchState_.wholeTrack = {
        -13.7,
        -10.5,
        -3.2,
        preview::EvidenceQuality::capturedStable,
        "Validated whole-track evidence"
    };
    updateLevelMatchSummary();
}

void ApprovedRefHubPreview::configureButton(juce::TextButton& button, const juce::String& tooltip)
{
    addAndMakeVisible(button);
    button.setWantsKeyboardFocus(true);
    button.setTooltip(tooltip);
}

void ApprovedRefHubPreview::updateLevelMatchSummary()
{
    const auto resolved = preview::resolveLevelMatch(levelMatchState_);
    if (!resolved.enabled || !resolved.measurement.mixLufs.has_value()
        || !resolved.measurement.referenceLufs.has_value()
        || !resolved.measurement.referenceMonitorTrimDb.has_value()) {
        levelMatchSummary_.setText(
            juce::String(preview::levelMatchModeName(resolved.effectiveMode)) + " · unavailable/off",
            juce::dontSendNotification);
        return;
    }

    levelMatchSummary_.setText(
        juce::String(preview::levelMatchModeName(resolved.effectiveMode))
            + " · Mix " + juce::String(*resolved.measurement.mixLufs, 1) + " LUFS"
            + " · Ref " + juce::String(*resolved.measurement.referenceLufs, 1) + " LUFS"
            + " · Ref trim " + juce::String(*resolved.measurement.referenceMonitorTrimDb, 1) + " dB",
        juce::dontSendNotification);
}

void ApprovedRefHubPreview::paint(juce::Graphics& g)
{
    g.fillAll(c(ui::colour::canvas));
    auto all = getLocalBounds().reduced(pad);

    auto header = all.removeFromTop(headerHeight);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(22.0f, true));
    g.drawText("Ehvota", header.removeFromLeft(92), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::liveMix));
    g.setFont(ui::font(16.0f, true));
    g.drawText("RefHub", header.removeFromLeft(64), juce::Justification::centredLeft);

    g.setColour(c(ui::colour::positive));
    g.fillEllipse(166.0f, 46.0f, 6.0f, 6.0f);
    g.setFont(ui::font(10.5f, true));
    g.drawText("LIVE CONNECTED · 24 SOURCES", 177, 38, 190, 22, juce::Justification::centredLeft);

    auto rail = all.removeFromLeft(railWidth);
    drawRoundPanel(g, rail);
    const std::array<const char*, 4> nav { "COMPARE", "SOURCES", "PROCESS", "HISTORY" };
    auto navY = rail.getY() + 24;
    for (std::size_t i = 0; i < nav.size(); ++i) {
        const auto active = i == 0;
        auto r = juce::Rectangle<int>(rail.getX() + 6, navY, rail.getWidth() - 12, 74);
        if (active) {
            g.setColour(c(ui::colour::liveMix).withAlpha(0.10f));
            g.fillRoundedRectangle(r.toFloat(), 8.0f);
            g.setColour(c(ui::colour::liveMix));
            g.fillRect(r.getX(), r.getY(), 3, r.getHeight());
        }
        g.setColour(active ? c(ui::colour::liveMix) : c(ui::colour::textSecondary));
        g.setFont(ui::font(11.0f, active));
        g.drawText(nav[i], r.reduced(8), juce::Justification::centred);
        navY += 90;
    }

    all.removeFromLeft(10);
    auto footer = all.removeFromBottom(24);
    g.setColour(c(ui::colour::positive));
    g.fillEllipse(static_cast<float>(footer.getX()), static_cast<float>(footer.getCentreY() - 3), 6.0f, 6.0f);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.5f));
    g.drawText("Ableton Connected    ·    Session: Ehvota Session    ·    Tempo 89.00    ·    4 / 4    ·    44.1 kHz",
        footer.withTrimmedLeft(12), juce::Justification::centredLeft);

    all.removeFromBottom(8);
    auto timeline = all.removeFromTop(timelineHeight);
    drawRoundPanel(g, timeline);
    auto tl = timeline.reduced(18, 12);
    auto sectionLabels = tl.removeFromTop(24);
    const std::array<const char*, 6> sections { "Intro", "Verse 1", "Chorus 1", "Verse 2", "Chorus 2", "Outro" };
    const auto sectionWidth = sectionLabels.getWidth() / static_cast<int>(sections.size());
    for (std::size_t i = 0; i < sections.size(); ++i) {
        auto sr = sectionLabels.removeFromLeft(sectionWidth);
        if (i == 2) {
            g.setColour(c(ui::colour::reference).withAlpha(0.14f));
            g.fillRoundedRectangle(sr.toFloat(), 4.0f);
        }
        g.setColour(i == 2 || i == 4 ? c(ui::colour::lowConfidence) : c(ui::colour::textSecondary));
        g.setFont(ui::font(10.5f, i == 2));
        g.drawText(sections[i], sr, juce::Justification::centred);
    }

    auto waves = tl.removeFromTop(74);
    auto mixWave = waves.removeFromTop(34).toFloat();
    auto refWave = waves.removeFromTop(34).toFloat();
    g.setColour(c(ui::colour::liveMix));
    g.setFont(ui::font(10.0f, true));
    g.drawText("YOUR MIX", timeline.getX() + 10, static_cast<int>(mixWave.getY()), 70, 26, juce::Justification::centredLeft);
    g.setColour(c(ui::colour::lowConfidence));
    g.drawText("REFERENCE", timeline.getX() + 10, static_cast<int>(refWave.getY()), 70, 26, juce::Justification::centredLeft);
    drawSectionWave(g, mixWave.withTrimmedLeft(76.0f), c(ui::colour::liveMix), 0.2f, 1.0f);
    drawSectionWave(g, refWave.withTrimmedLeft(76.0f), c(ui::colour::lowConfidence), 0.6f, 0.86f);

    const auto selectedX = waves.getX() + static_cast<int>((waves.getWidth() - 76) * 2.0 / 6.0) + 76;
    const auto selectedW = static_cast<int>((waves.getWidth() - 76) / 6.0);
    g.setColour(c(ui::colour::reference).withAlpha(0.12f));
    g.fillRect(selectedX, static_cast<int>(mixWave.getY()), selectedW, static_cast<int>(refWave.getBottom() - mixWave.getY()));
    g.setColour(c(ui::colour::reference));
    g.drawRect(selectedX, static_cast<int>(mixWave.getY()), selectedW, static_cast<int>(refWave.getBottom() - mixWave.getY()), 1);

    auto timelineInfo = tl;
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::numericFont(15.0f, true));
    g.drawText("01:32.456", timelineInfo.removeFromLeft(150), juce::Justification::centred);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::numericFont(10.5f));
    g.drawText("LOOP  01:12.000 – 01:48.000 (36.000 s)", timelineInfo.removeFromLeft(310), juce::Justification::centredLeft);
    g.drawText("CAPTURE  00:24 / 00:36", timelineInfo, juce::Justification::centredLeft);

    all.removeFromTop(10);
    auto insight = all.removeFromBottom(insightHeight);
    all.removeFromBottom(10);
    drawRoundPanel(g, insight);

    const auto inspectorWidth = inspectorOpen_ && all.getWidth() > 980 ? 238 : 0;
    auto inspector = inspectorWidth > 0 ? all.removeFromRight(inspectorWidth) : juce::Rectangle<int>();
    if (inspectorWidth > 0)
        all.removeFromRight(10);

    drawRoundPanel(g, all);
    auto analyser = all.reduced(18, 12);
    auto tabs = analyser.removeFromTop(34);
    const std::array<const char*, 4> lenses { "TONE", "DYNAMICS", "SPACE", "COMPONENTS" };
    for (std::size_t i = 0; i < lenses.size(); ++i) {
        auto tr = tabs.removeFromLeft(i == 0 ? 92 : 118);
        g.setColour(i == 0 ? c(ui::colour::liveMix) : c(ui::colour::textSecondary));
        g.setFont(ui::font(11.0f, i == 0));
        g.drawText(lenses[i], tr, juce::Justification::centred);
        if (i == 0)
            g.fillRect(tr.getX() + 8, tr.getBottom() - 2, tr.getWidth() - 16, 2);
    }

    auto graph = analyser.reduced(2, 6).toFloat();
    g.setColour(c(ui::colour::borderSubtle).withAlpha(0.45f));
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
    g.setColour(c(ui::colour::proposed).withAlpha(0.10f));
    g.fillRect(juce::Rectangle<float>(selectedStart, graph.getY(), selectedEnd - selectedStart, graph.getHeight()));
    g.setColour(c(ui::colour::proposed).withAlpha(0.9f));
    g.drawVerticalLine(static_cast<int>(selectedStart), graph.getY(), graph.getBottom());
    g.drawVerticalLine(static_cast<int>(selectedEnd), graph.getY(), graph.getBottom());

    drawSpectrum(g, graph, c(ui::colour::liveMix), 0.12f, 0.0f, 0.10f);
    drawSpectrum(g, graph, c(ui::colour::reference), -0.18f, 1.2f, 0.08f);

    auto callout = juce::Rectangle<int>(static_cast<int>(selectedStart + 28), analyser.getY() + 42, 206, 120);
    g.setColour(c(ui::colour::overlay).withAlpha(0.98f));
    g.fillRoundedRectangle(callout.toFloat(), 8.0f);
    g.setColour(c(ui::colour::proposed));
    g.drawRoundedRectangle(callout.toFloat().reduced(0.5f), 8.0f, 1.0f);
    auto ct = callout.reduced(14, 10);
    g.setFont(ui::font(10.0f, true));
    g.drawText("LOW-MID BUMP", ct.removeFromTop(20), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::numericFont(19.0f, true));
    g.drawText("220 – 410 Hz", ct.removeFromTop(27), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::proposed));
    g.setFont(ui::numericFont(20.0f, true));
    g.drawText("+1.8 dB", ct.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(11.0f));
    g.drawText("vs Reference · Confidence: High", ct, juce::Justification::centredLeft);

    if (inspectorWidth > 0) {
        drawRoundPanel(g, inspector);
        auto ir = inspector.reduced(14);
        g.setColour(c(ui::colour::textPrimary));
        g.setFont(ui::font(12.0f, true));
        g.drawText("INSPECTOR", ir.removeFromTop(28), juce::Justification::centredLeft);
        g.setColour(c(ui::colour::proposed));
        g.setFont(ui::numericFont(19.0f, true));
        g.drawText("220 – 410 Hz", ir.removeFromTop(34), juce::Justification::centredLeft);
        g.setColour(c(ui::colour::textSecondary));
        g.setFont(ui::numericFont(11.0f));
        for (const auto* line : { "Your Mix      -3.2 dB", "Reference     -5.0 dB", "Difference    +1.8 dB", "Confidence    High", "", "Main contributors", "Drums         High", "Bass          High", "Music         Medium", "Vocals        Low" }) {
            g.drawText(line, ir.removeFromTop(21), juce::Justification::centredLeft);
        }
    }

    auto ins = insight.reduced(18, 12);
    auto finding = ins.removeFromLeft(static_cast<int>(ins.getWidth() * 0.36));
    g.setColour(c(ui::colour::proposed));
    g.setFont(ui::font(10.0f, true));
    g.drawText("EHVOTA INSIGHT", finding.removeFromTop(22), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(19.0f, true));
    g.drawText("Low-mid buildup", finding.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(11.0f));
    g.drawFittedText("The low-mid range (220–410 Hz) is consistently louder than the reference in this section. Mainly Drums + Bass. High confidence.", finding.removeFromTop(50), juce::Justification::topLeft, 3);

    auto focus = ins.removeFromLeft(static_cast<int>(ins.getWidth() * 0.38));
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.0f, true));
    g.drawText("SUGGESTED FOCUS", focus.removeFromTop(20), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(11.0f));
    g.drawText("Dynamic Control    ·    Surgical EQ    ·    Tone Shaper", focus.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textMuted));
    g.drawFittedText("These are analysis directions only. Use CREATE PROPOSAL to review exact processor changes before anything can be applied.", focus, juce::Justification::topLeft, 3);

    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.0f, true));
    g.drawText("QUICK ACTIONS", ins.removeFromTop(20), juce::Justification::centredLeft);
}

void ApprovedRefHubPreview::resized()
{
    auto all = getLocalBounds().reduced(pad);
    auto header = all.removeFromTop(headerHeight);

    header.removeFromLeft(370);
    auto refArea = header.removeFromLeft(170).reduced(3, 10);
    referenceSelector_.setBounds(refArea);
    auto sectionArea = header.removeFromLeft(160).reduced(3, 10);
    sectionSelector_.setBounds(sectionArea);
    header.removeFromLeft(12);
    mixButton_.setBounds(header.removeFromLeft(72).reduced(2, 10));
    referenceButton_.setBounds(header.removeFromLeft(72).reduced(2, 10));
    header.removeFromLeft(12);
    blindButton_.setBounds(header.removeFromLeft(82).reduced(2, 10));
    captureButton_.setBounds(header.removeFromLeft(94).reduced(2, 10));
    header.removeFromLeft(8);
    levelMatchMode_.setBounds(header.removeFromLeft(132).reduced(2, 10));
    levelMatchSummary_.setBounds(header.reduced(6, 10));

    all.removeFromLeft(railWidth + 10);
    all.removeFromBottom(32);
    all.removeFromTop(timelineHeight + 10);
    auto insight = all.removeFromBottom(insightHeight);
    all.removeFromBottom(10);

    const auto inspectorWidth = inspectorOpen_ && all.getWidth() > 980 ? 238 : 0;
    if (inspectorWidth > 0) {
        auto inspector = all.removeFromRight(inspectorWidth);
        inspectorButton_.setBounds(inspector.removeFromTop(32).removeFromRight(92).reduced(2));
        auto actionArea = inspector.removeFromBottom(108).reduced(12, 4);
        listenRangeButton_.setBounds(actionArea.removeFromTop(30));
        actionArea.removeFromTop(4);
        compareButton_.setBounds(actionArea.removeFromTop(30));
        actionArea.removeFromTop(4);
        createProposalButton_.setBounds(actionArea.removeFromTop(30));
    }
    else {
        inspectorButton_.setBounds(getLocalBounds().removeFromTop(80).removeFromRight(110).reduced(8, 20));
        listenRangeButton_.setBounds({});
        compareButton_.setBounds({});
        createProposalButton_.setBounds({});
    }

    // Reserve a future direct-action row inside the Insight region. Current approval preview
    // keeps the action semantics painted and proposal-only until the Sources/Process screens land.
    juce::ignoreUnused(insight);
}

} // namespace ehvota::previewapp
