#include "ShellPresentation.h"

#include <array>
#include <cmath>
#include <stdexcept>

namespace ehvota::preview {

ShellDensity classifyShellDensity(const int logicalWidth)
{
    if (logicalWidth <= 0)
        throw std::invalid_argument("Shell width must be greater than zero.");
    if (logicalWidth < 820)
        return ShellDensity::compact;
    if (logicalWidth < 1'120)
        return ShellDensity::standard;
    return ShellDensity::wide;
}

std::vector<std::string> validateShellPresentation(
    const ShellPresentation& presentation)
{
    std::vector<std::string> errors;
    if (presentation.productName.empty())
        errors.emplace_back("Shell product name is empty.");
    if (presentation.presetName.empty())
        errors.emplace_back("Shell preset name is empty.");
    if (presentation.statusText.empty())
        errors.emplace_back("Shell status text is empty.");
    if (presentation.showProcessingAction
        && presentation.processingActionLabel.empty()) {
        errors.emplace_back("Visible shell processing action has an empty label.");
    }

    constexpr std::array allowedScalePercentages { 75, 100, 125, 150, 175, 200 };
    bool scaleAllowed = false;
    for (const auto allowed : allowedScalePercentages)
        scaleAllowed = scaleAllowed || presentation.scalePercent == allowed;
    if (!scaleAllowed)
        errors.emplace_back("Shell scale percentage is unsupported.");

    if (!std::isfinite(presentation.outputLevelDb)
        || presentation.outputLevelDb < -100.0
        || presentation.outputLevelDb > 12.0) {
        errors.emplace_back("Shell output level is outside the display range.");
    }

    return errors;
}

} // namespace ehvota::preview
