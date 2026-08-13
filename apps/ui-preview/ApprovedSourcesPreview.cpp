#include "ApprovedSourcesPreview.h"

#include "shared/ui/Theme.h"

#include <array>

namespace ehvota::previewapp {
namespace {

juce::Colour c(const juce::uint32 value) { return juce::Colour(value); }

void panel(juce::Graphics& g, juce::Rectangle<int> area, const bool raised = false)
{
    g.setColour(c(raised ? ehvota::ui::colour::raised : ehvota::ui::colour::base));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);
    g.setColour(c(ehvota::ui::colour::borderSubtle));
    g.drawRoundedRectangle(area.toFloat().reduced(0.5f), 8.0f, 1.0f);
}

void statusDot(juce::Graphics& g, const juce::Rectangle<int> area, const juce::Colour colour)
{
    g.setColour(colour);
    g.fillEllipse(static_cast<float>(area.getX()), static_cast<float>(area.getCentreY() - 3), 6.0f, 6.0f);
}

} // namespace

ApprovedSourcesPreview::ApprovedSourcesPreview()
{
    for (auto* button : { &validateButton_, &refreshButton_, &saveMappingButton_ }) {
        addAndMakeVisible(*button);
        button->setWantsKeyboardFocus(true);
    }
    validateButton_.setTooltip("Validate that every reference role points to the intended Ableton/Ehvota source.");
    refreshButton_.setTooltip("Refresh read-only Ableton Live Set discovery through Ehvota Bridge.");
    saveMappingButton_.setTooltip("Save the confirmed semantic mapping. Track names alone are never treated as truth.");
}

void ApprovedSourcesPreview::paint(juce::Graphics& g)
{
    g.fillAll(c(ui::colour::canvas));
    auto bounds = getLocalBounds().reduced(14);

    auto header = bounds.removeFromTop(64);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(22.0f, true));
    g.drawText("Ehvota", header.removeFromLeft(92), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::liveMix));
    g.setFont(ui::font(16.0f, true));
    g.drawText("RefHub", header.removeFromLeft(66), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(11.0f, true));
    g.drawText("SOURCES / MAPPING", header.removeFromLeft(180), juce::Justification::centredLeft);

    statusDot(g, header, c(ui::colour::positive));
    g.setColour(c(ui::colour::positive));
    g.setFont(ui::font(10.5f, true));
    g.drawText("  LIVE CONNECTED · 5 MAPPABLE SOURCES", header, juce::Justification::centredLeft);

    auto tabs = bounds.removeFromTop(38);
    const std::array<const char*, 4> names { "LIBRARY", "MAPPING", "SECTIONS", "VALIDATION" };
    for (std::size_t i = 0; i < names.size(); ++i) {
        auto tab = tabs.removeFromLeft(126);
        g.setColour(i == 1 ? c(ui::colour::liveMix) : c(ui::colour::textSecondary));
        g.setFont(ui::font(11.0f, i == 1));
        g.drawText(names[i], tab, juce::Justification::centred);
        if (i == 1)
            g.fillRect(tab.getX() + 12, tab.getBottom() - 2, tab.getWidth() - 24, 2);
    }

    bounds.removeFromTop(10);
    auto footer = bounds.removeFromBottom(88);
    bounds.removeFromBottom(10);

    constexpr int gap = 10;
    const auto totalWidth = bounds.getWidth();
    const auto sideWidth = static_cast<int>(totalWidth * 0.35f);
    const auto centreWidth = totalWidth - sideWidth * 2 - gap * 2;

    auto left = bounds.removeFromLeft(sideWidth);
    bounds.removeFromLeft(gap);
    auto centre = bounds.removeFromLeft(centreWidth);
    bounds.removeFromLeft(gap);
    auto right = bounds;

    panel(g, left);
    panel(g, centre);
    panel(g, right);

    auto l = left.reduced(16);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(14.0f, true));
    g.drawText("REFERENCE SOURCES", l.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textMuted));
    g.setFont(ui::font(10.5f));
    g.drawText("Ayo Nights · full mix authoritative", l.removeFromTop(22), juce::Justification::centredLeft);

    const std::array<const char*, 5> stems { "full_mix.wav", "drums.wav", "bass.wav", "vocals.wav", "music.wav" };
    const std::array<const char*, 5> roles { "Full Mix", "Drums", "Bass", "Vocals", "Music" };
    for (std::size_t i = 0; i < stems.size(); ++i) {
        auto row = l.removeFromTop(58);
        panel(g, row, true);
        auto content = row.reduced(10, 6);
        g.setColour(i == 0 ? c(ui::colour::reference) : c(ui::colour::textPrimary));
        g.setFont(ui::font(11.5f, true));
        g.drawText(roles[i], content.removeFromTop(22), juce::Justification::centredLeft);
        g.setColour(c(ui::colour::textSecondary));
        g.setFont(ui::font(10.5f));
        g.drawText(stems[i], content, juce::Justification::centredLeft);
        l.removeFromTop(6);
    }

    auto m = centre.reduced(12);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(14.0f, true));
    g.drawText("MAPPING", m.removeFromTop(28), juce::Justification::centred);
    g.setColour(c(ui::colour::textMuted));
    g.setFont(ui::font(10.0f));
    g.drawFittedText("Semantic roles stay explicit. Renamed Ableton tracks do not silently remap.", m.removeFromTop(34), juce::Justification::centred, 2);

    const std::array<const char*, 5> mapState { "MASTER", "DRUM BUS", "BASS BUS", "VOCAL BUS", "MUSIC BUS" };
    for (std::size_t i = 0; i < mapState.size(); ++i) {
        auto row = m.removeFromTop(58);
        const auto isDisconnected = i == 3;
        g.setColour(c(isDisconnected ? ui::colour::proposed : ui::colour::borderSubtle));
        g.drawHorizontalLine(row.getCentreY(), static_cast<float>(row.getX() + 6), static_cast<float>(row.getRight() - 6));
        g.setColour(c(isDisconnected ? ui::colour::proposed : ui::colour::liveMix));
        g.fillEllipse(static_cast<float>(row.getCentreX() - 5), static_cast<float>(row.getCentreY() - 5), 10.0f, 10.0f);
        g.setColour(c(ui::colour::canvas));
        g.fillEllipse(static_cast<float>(row.getCentreX() - 2), static_cast<float>(row.getCentreY() - 2), 4.0f, 4.0f);

        auto badge = juce::Rectangle<int>(row.getX() + 14, row.getCentreY() - 11, row.getWidth() - 28, 22);
        g.setColour(c(isDisconnected ? ui::colour::proposed : ui::colour::raised));
        g.fillRoundedRectangle(badge.toFloat(), 11.0f);
        g.setColour(c(isDisconnected ? ui::colour::canvas : ui::colour::textPrimary));
        g.setFont(ui::font(9.5f, true));
        g.drawText(isDisconnected ? "RELINK REQUIRED" : mapState[i], badge, juce::Justification::centred);
        m.removeFromTop(6);
    }

    auto r = right.reduced(16);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(14.0f, true));
    g.drawText("ABLETON / EHVOTA SOURCES", r.removeFromTop(28), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textMuted));
    g.setFont(ui::font(10.5f));
    g.drawText("Live Set structure + measured Ehvota endpoints", r.removeFromTop(22), juce::Justification::centredLeft);

    const std::array<const char*, 5> live { "Master", "DRUM BUS", "BASS BUS", "VOCAL BUS (missing)", "MUSIC BUS" };
    const std::array<const char*, 5> endpoints { "RefHub", "RefTap · post", "RefTap · post", "Disconnected", "RefTap · post" };
    for (std::size_t i = 0; i < live.size(); ++i) {
        auto row = r.removeFromTop(58);
        panel(g, row, true);
        const auto disconnected = i == 3;
        statusDot(g, row.reduced(10, 0), c(disconnected ? ui::colour::proposed : ui::colour::positive));
        auto content = row.reduced(26, 6);
        g.setColour(c(disconnected ? ui::colour::proposed : ui::colour::textPrimary));
        g.setFont(ui::font(11.5f, true));
        g.drawText(live[i], content.removeFromTop(22), juce::Justification::centredLeft);
        g.setColour(c(ui::colour::textSecondary));
        g.setFont(ui::font(10.5f));
        g.drawText(endpoints[i], content, juce::Justification::centredLeft);
        r.removeFromTop(6);
    }

    panel(g, footer);
    auto f = footer.reduced(16, 10);
    auto summary = f.removeFromLeft(static_cast<int>(f.getWidth() * 0.48f));
    g.setColour(c(ui::colour::proposed));
    g.setFont(ui::font(10.0f, true));
    g.drawText("MAPPING NEEDS ATTENTION", summary.removeFromTop(20), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(11.5f, true));
    g.drawText("4 confirmed · 1 disconnected", summary.removeFromTop(22), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(10.0f));
    g.drawFittedText("Composite: Rhythm = drums.wav + bass.wav ↔ DRUM BUS + BASS BUS", summary, juce::Justification::topLeft, 2);
}

void ApprovedSourcesPreview::resized()
{
    auto bounds = getLocalBounds().reduced(14);
    bounds.removeFromTop(64 + 38 + 10);
    auto footer = bounds.removeFromBottom(88);
    auto actions = footer.reduced(16, 22);
    refreshButton_.setBounds(actions.removeFromRight(150));
    actions.removeFromRight(8);
    saveMappingButton_.setBounds(actions.removeFromRight(140));
    actions.removeFromRight(8);
    validateButton_.setBounds(actions.removeFromRight(160));
}

} // namespace ehvota::previewapp
