#pragma once

#include "DesignTokenValues.h"
#include "shared/viewmodels/PreviewScenario.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace ehvota::ui {

namespace colour {
inline constexpr juce::uint32 canvas = token::canvas;
inline constexpr juce::uint32 base = token::base;
inline constexpr juce::uint32 raised = token::raised;
inline constexpr juce::uint32 overlay = token::overlay;
inline constexpr juce::uint32 borderSubtle = token::borderSubtle;
inline constexpr juce::uint32 borderStrong = token::borderStrong;
inline constexpr juce::uint32 textPrimary = token::textPrimary;
inline constexpr juce::uint32 textSecondary = token::textSecondary;
inline constexpr juce::uint32 textMuted = token::textMuted;
inline constexpr juce::uint32 focus = token::focus;
inline constexpr juce::uint32 liveMix = token::liveMix;
inline constexpr juce::uint32 reference = token::reference;
inline constexpr juce::uint32 proposed = token::proposed;
inline constexpr juce::uint32 processing = token::processing;
inline constexpr juce::uint32 positive = token::positive;
inline constexpr juce::uint32 caution = token::caution;
inline constexpr juce::uint32 error = token::error;
inline constexpr juce::uint32 lowConfidence = token::lowConfidence;
} // namespace colour

[[nodiscard]] juce::Colour toneColour(preview::Tone tone);
[[nodiscard]] juce::Font font(float height, bool semibold = false);
[[nodiscard]] juce::Font numericFont(float height, bool semibold = true);

} // namespace ehvota::ui
