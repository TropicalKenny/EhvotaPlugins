#include "ApprovedSourcesPreview.h"

#include "shared/ui/Theme.h"

#include <array>

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
    g.drawText("Ehvota RefHub", header.removeFromLeft(180), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textSecondary));
    g.setFont(ui::font(12.0f));
    g.drawText("SOURCES  /  MAPPING", header.removeFromLeft(190), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::positive));
    g.fillEllipse(static_cast<float>(header.getX()), static_cast<float>(header.getCentreY() - 3), 6.0f, 6.0f);
    g.drawText("  Ableton Bridge connected · Live 12 Suite", header, juce::Justification::centredLeft);

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
    auto footer = bounds.removeFromBottom(78);
    bounds.removeFromBottom(10);

    const auto gap = 12;
    auto left = bounds.removeFromLeft((bounds.getWidth() - gap) / 2);
    bounds.removeFromLeft(gap);
    auto right = bounds;
    panel(g, left);
    panel(g, right);

    auto l = left.reduced(18);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(15.0f, true));
    g.drawText("REFERENCE SOURCES", l.removeFromTop(30), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textMuted));
    g.setFont(ui::font(11.0f));
    g.drawText("Ayo Nights · full mix authoritative", l.removeFromTop(24), juce::Justification::centredLeft);

    const std::array<const char*, 5> stems { "full_mix.wav", "drums.wav", "bass.wav", "vocals.wav", "music.wav" };
    const std::array<const char*, 5> roles { "Full Mix", "Drums", "Bass", "Vocals", "Music" };
    for (std::size_t i = 0; i < stems.size(); ++i) {
        auto row = l.removeFromTop(56);
        g.setColour(c(ui::colour::raised));
        g.fillRoundedRectangle(row.toFloat(), 6.0f);
        g.setColour(i == 0 ? c(ui::colour::reference) : c(ui::colour::textPrimary));
        g.setFont(ui::font(12.0f, true));
        g.drawText(roles[i], row.removeFromLeft(92).reduced(10, 0), juce::Justification::centredLeft);
        g.setColour(c(ui::colour::textSecondary));
        g.setFont(ui::font(11.0f));
        g.drawText(stems[i], row.reduced(8, 0), juce::Justification::centredLeft);
        l.removeFromTop(6);
    }

    auto r = right.reduced(18);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(15.0f, true));
    g.drawText("ABLETON / EHVOTA SOURCES", r.removeFromTop(30), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textMuted));
    g.setFont(ui::font(11.0f));
    g.drawText("Discovered structure + measured Ehvota endpoints", r.removeFromTop(24), juce::Justification::centredLeft);

    const std::array<const char*, 5> live { "Master", "DRUM BUS", "BASS BUS", "VOCAL BUS", "MUSIC BUS" };
    const std::array<const char*, 5> endpoints { "RefHub", "RefTap · post", "RefTap · post", "RefComp · post", "RefTap · post" };
    for (std::size_t i = 0; i < live.size(); ++i) {
        auto row = r.removeFromTop(56);
        g.setColour(c(ui::colour::raised));
        g.fillRoundedRectangle(row.toFloat(), 6.0f);
        g.setColour(c(ui::colour::positive));
        g.fillEllipse(static_cast<float>(row.getX() + 10), static_cast<float>(row.getCentreY() - 3), 6.0f, 6.0f);
        g.setColour(c(ui::colour::textPrimary));
        g.setFont(ui::font(12.0f, true));
        g.drawText(live[i], row.removeFromLeft(138).withTrimmedLeft(24), juce::Justification::centredLeft);
        g.setColour(c(ui::colour::textSecondary));
        g.setFont(ui::font(11.0f));
        g.drawText(endpoints[i], row.reduced(8, 0), juce::Justification::centredLeft);
        r.removeFromTop(6);
    }

    auto f = footer;
    panel(g, f);
    f = f.reduced(18, 12);
    g.setColour(c(ui::colour::proposed));
    g.setFont(ui::font(11.0f, true));
    g.drawText("COMPOSITE", f.removeFromLeft(90), juce::Justification::centredLeft);
    g.setColour(c(ui::colour::textPrimary));
    g.setFont(ui::font(12.0f, true));
    g.drawText("Rhythm: drums.wav + bass.wav  ↔  DRUM BUS + BASS BUS", f, juce::Justification::centredLeft);
}

void ApprovedSourcesPreview::resized()
{
    auto bounds = getLocalBounds().reduced(14);
    bounds.removeFromTop(64 + 38 + 10);
    auto footer = bounds.removeFromBottom(78);
    auto actions = footer.reduced(18, 20);
    refreshButton_.setBounds(actions.removeFromRight(150));
    actions.removeFromRight(8);
    saveMappingButton_.setBounds(actions.removeFromRight(140));
    actions.removeFromRight(8);
    validateButton_.setBounds(actions.removeFromRight(160));
}

} // namespace ehvota::previewapp
