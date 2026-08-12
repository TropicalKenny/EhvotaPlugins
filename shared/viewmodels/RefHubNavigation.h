#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ehvota::preview {

enum class RefHubPrimaryView {
    compare,
    sources,
    process,
    history,
    settings
};

struct RefHubNavigationState {
    RefHubPrimaryView primary { RefHubPrimaryView::compare };
    std::string secondary { "Tone" };
};

[[nodiscard]] const std::vector<std::string>& primaryRefHubViewNames();
[[nodiscard]] const std::vector<std::string>& secondaryRefHubViewNames(RefHubPrimaryView primary);
[[nodiscard]] RefHubPrimaryView primaryViewForLegacyView(std::string_view view);
[[nodiscard]] std::string defaultSecondaryView(RefHubPrimaryView primary);
[[nodiscard]] RefHubNavigationState navigationForScenarioView(std::string_view legacyOrApprovedView);

} // namespace ehvota::preview
