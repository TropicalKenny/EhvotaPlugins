#include "Components.h"

#include "Theme.h"

#include <cmath>
#include <functional>

namespace ehvota::ui {

StatusBadge::StatusBadge(preview::Badge badge)
    : badge_(std::move(badge))
{
    setInterceptsMouseClicks(false, false);
}

void StatusBadge::paint(juce::Graphics& graphics)
{
    const auto accent = toneColour(badge_.tone);
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);

    graphics.setColour(accent.withAlpha(0.13f));
    graphics.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f);
    graphics.setColour(accent.withAlpha(0.75f));
    graphics.drawRoundedRectangle(bounds, bounds.getHeight() * 0.5f, 1.0f);
    graphics.setColour(accent);
    graphics.setFont(font(11.0f, true));
    graphics.drawText(badge_.text, getLocalBounds().reduced(10, 0), juce::Justification::centred);
}

int StatusBadge::idealWidth() const
{
    return juce::jmax(76, 22 + static_cast<int>(badge_.text.size()) * 7);
}

MetricPanel::MetricPanel(preview::Panel panel)
    : panel_(std::move(panel))
{
    setInterceptsMouseClicks(false, false);
}

void MetricPanel::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    graphics.setColour(juce::Colour(colour::raised));
    graphics.fillRoundedRectangle(bounds, 8.0f);
    graphics.setColour(juce::Colour(colour::borderSubtle));
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    auto content = getLocalBounds().reduced(16, 13);
    auto heading = content.removeFromTop(22);
    graphics.setFont(font(15.0f, true));
    graphics.setColour(juce::Colour(colour::textPrimary));
    graphics.drawText(panel_.title, heading, juce::Justification::centredLeft);

    auto summary = content.removeFromTop(34);
    graphics.setFont(font(12.0f));
    graphics.setColour(juce::Colour(colour::textSecondary));
    graphics.drawFittedText(panel_.summary, summary, juce::Justification::topLeft, 2);

    for (const auto& metric : panel_.metrics) {
        auto row = content.removeFromTop(46);
        auto labelArea = row.removeFromLeft(juce::jmax(104, row.getWidth() / 3));

        graphics.setFont(font(12.0f, true));
        graphics.setColour(juce::Colour(colour::textSecondary));
        graphics.drawText(metric.label, labelArea.removeFromTop(19), juce::Justification::centredLeft);

        graphics.setFont(numericFont(16.0f, true));
        graphics.setColour(toneColour(metric.tone));
        graphics.drawText(metric.value, row.removeFromTop(22), juce::Justification::centredLeft);

        graphics.setFont(font(11.0f));
        graphics.setColour(juce::Colour(colour::textMuted));
        graphics.drawFittedText(metric.detail, row, juce::Justification::centredLeft, 1);

        content.removeFromTop(3);
    }
}

int MetricPanel::preferredHeight() const
{
    return 82 + static_cast<int>(panel_.metrics.size()) * 49;
}

void SignalGraph::setScenario(std::string scenarioId)
{
    scenarioId_ = std::move(scenarioId);
    repaint();
}

void SignalGraph::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat();
    graphics.setColour(juce::Colour(colour::base));
    graphics.fillRoundedRectangle(bounds, 8.0f);
    graphics.setColour(juce::Colour(colour::borderSubtle));
    graphics.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    auto plot = bounds.reduced(14.0f, 18.0f);
    graphics.setColour(juce::Colour(colour::borderSubtle).withAlpha(0.45f));
    for (int index = 1; index < 6; ++index) {
        const auto x = plot.getX() + plot.getWidth() * static_cast<float>(index) / 6.0f;
        graphics.drawVerticalLine(static_cast<int>(x), plot.getY(), plot.getBottom());
    }
    for (int index = 1; index < 4; ++index) {
        const auto y = plot.getY() + plot.getHeight() * static_cast<float>(index) / 4.0f;
        graphics.drawHorizontalLine(static_cast<int>(y), plot.getX(), plot.getRight());
    }

    const auto seed = std::hash<std::string> {}(scenarioId_);
    auto makePath = [&](const float phase, const float bias) {
        juce::Path path;
        constexpr int points = 96;
        for (int index = 0; index < points; ++index) {
            const auto ratio = static_cast<float>(index) / static_cast<float>(points - 1);
            const auto wave = 0.20f * std::sin(ratio * 12.0f + phase)
                + 0.09f * std::sin(ratio * 31.0f + phase * 0.6f)
                + 0.05f * std::sin(ratio * 67.0f + static_cast<float>(seed % 17));
            const auto x = plot.getX() + ratio * plot.getWidth();
            const auto y = plot.getCentreY() - (wave + bias) * plot.getHeight();
            if (index == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }
        return path;
    };

    graphics.setColour(juce::Colour(colour::liveMix));
    graphics.strokePath(makePath(0.2f, 0.04f), juce::PathStrokeType(2.0f));

    const float dashLengths[] { 7.0f, 5.0f };
    juce::Path dashedReference;
    juce::PathStrokeType(1.7f).createDashedStroke(
        dashedReference,
        makePath(0.75f, -0.03f),
        dashLengths,
        2);
    graphics.setColour(juce::Colour(colour::reference));
    graphics.fillPath(dashedReference);

    graphics.setFont(font(11.0f, true));
    graphics.setColour(juce::Colour(colour::liveMix));
    graphics.drawText("MIX — SOLID", plot.toNearestInt().removeFromTop(18), juce::Justification::topLeft);
    graphics.setColour(juce::Colour(colour::reference));
    graphics.drawText("REF -- DASHED", plot.toNearestInt().removeFromTop(18), juce::Justification::topRight);

}

} // namespace ehvota::ui
