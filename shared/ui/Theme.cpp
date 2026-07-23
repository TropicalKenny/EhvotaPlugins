#include "Theme.h"

namespace ehvota::ui {

juce::Colour toneColour(const preview::Tone tone)
{
    using enum preview::Tone;
    switch (tone) {
    case liveMix:
        return juce::Colour(colour::liveMix);
    case reference:
        return juce::Colour(colour::reference);
    case proposed:
        return juce::Colour(colour::proposed);
    case positive:
        return juce::Colour(colour::positive);
    case caution:
        return juce::Colour(colour::caution);
    case error:
        return juce::Colour(colour::error);
    case lowConfidence:
        return juce::Colour(colour::lowConfidence);
    case neutral:
        return juce::Colour(colour::textSecondary);
    }

    return juce::Colour(colour::textSecondary);
}

juce::Font font(const float height, const bool semibold)
{
    return juce::Font {
        juce::FontOptions {
            "Segoe UI",
            height,
            semibold ? juce::Font::bold : juce::Font::plain
        }
    };
}

juce::Font numericFont(const float height, const bool semibold)
{
    return juce::Font {
        juce::FontOptions {
            "Segoe UI",
            height,
            semibold ? juce::Font::bold : juce::Font::plain
        }
            .withFeatureEnabled("tnum")
            .withKerningFactor(0.01f)
    };
}

} // namespace ehvota::ui
