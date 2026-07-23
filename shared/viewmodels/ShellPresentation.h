#pragma once

#include <string>
#include <vector>

namespace ehvota::preview {

enum class ShellDensity {
    compact,
    standard,
    wide
};

struct ShellPresentation {
    std::string productName;
    std::string presetName;
    std::string processingActionLabel;
    std::string statusText;
    std::string statusDetail;
    bool canUndo {};
    bool canRedo {};
    bool comparisonB {};
    bool processingActive {};
    bool showProcessingAction {};
    bool processingActionEnabled { true };
    bool showOutputMeter {};
    bool statusIsWarning {};
    int scalePercent { 100 };
    double outputLevelDb { -18.0 };
};

[[nodiscard]] ShellDensity classifyShellDensity(int logicalWidth);
[[nodiscard]] std::vector<std::string> validateShellPresentation(
    const ShellPresentation& presentation);

} // namespace ehvota::preview
