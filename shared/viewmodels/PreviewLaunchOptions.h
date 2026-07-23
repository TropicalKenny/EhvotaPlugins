#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ehvota::preview {

inline constexpr int minimumLaunchWidth = 720;
inline constexpr int minimumLaunchHeight = 500;
inline constexpr int maximumLaunchWidth = 1920;
inline constexpr int maximumLaunchHeight = 1200;
inline constexpr int defaultLaunchWidth = 1180;
inline constexpr int defaultLaunchHeight = 760;
inline constexpr int minimumLaunchStep = 1;
inline constexpr int maximumLaunchStep = 32;

struct LaunchOptions {
    std::string scenarioId;
    int contentWidth { defaultLaunchWidth };
    int contentHeight { defaultLaunchHeight };
    std::optional<int> journeyStep;
};

struct LaunchOptionsResult {
    std::optional<LaunchOptions> options;
    std::string error;
};

[[nodiscard]] const std::string& defaultLaunchScenarioId();
[[nodiscard]] LaunchOptionsResult parseLaunchOptions(const std::vector<std::string>& arguments);

} // namespace ehvota::preview
