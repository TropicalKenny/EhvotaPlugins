#include "RefHubNavigation.h"

#include <array>

namespace ehvota::preview {
namespace {

const std::vector<std::string> primaryNames {
    "Compare",
    "Sources",
    "Process",
    "History",
    "Settings"
};

const std::vector<std::string> compareSecondary {
    "Overview",
    "Tone",
    "Dynamics",
    "Space",
    "Components"
};

const std::vector<std::string> sourcesSecondary {
    "Library",
    "Mapping",
    "Sections",
    "Validation"
};

const std::vector<std::string> processSecondary {
    "Links",
    "Proposals",
    "Signal Chain",
    "Learn"
};

const std::vector<std::string> historySecondary {
    "Captures",
    "Snapshots"
};

const std::vector<std::string> settingsSecondary {
    "General",
    "Analysis",
    "Ableton",
    "Diagnostics"
};

bool contains(const std::vector<std::string>& values, const std::string_view value)
{
    for (const auto& candidate : values) {
        if (candidate == value)
            return true;
    }
    return false;
}

} // namespace

const std::vector<std::string>& primaryRefHubViewNames()
{
    return primaryNames;
}

const std::vector<std::string>& secondaryRefHubViewNames(const RefHubPrimaryView primary)
{
    switch (primary) {
    case RefHubPrimaryView::compare:
        return compareSecondary;
    case RefHubPrimaryView::sources:
        return sourcesSecondary;
    case RefHubPrimaryView::process:
        return processSecondary;
    case RefHubPrimaryView::history:
        return historySecondary;
    case RefHubPrimaryView::settings:
        return settingsSecondary;
    }

    return compareSecondary;
}

RefHubPrimaryView primaryViewForLegacyView(const std::string_view view)
{
    if (contains(compareSecondary, view) || view == "Compare" || view == "Bands")
        return RefHubPrimaryView::compare;

    if (contains(sourcesSecondary, view))
        return RefHubPrimaryView::sources;

    if (contains(processSecondary, view))
        return RefHubPrimaryView::process;

    if (view == "History")
        return RefHubPrimaryView::history;

    if (view == "Settings" || view == "Timing")
        return RefHubPrimaryView::settings;

    return RefHubPrimaryView::compare;
}

std::string defaultSecondaryView(const RefHubPrimaryView primary)
{
    switch (primary) {
    case RefHubPrimaryView::compare:
        return "Tone";
    case RefHubPrimaryView::sources:
        return "Library";
    case RefHubPrimaryView::process:
        return "Links";
    case RefHubPrimaryView::history:
        return "Captures";
    case RefHubPrimaryView::settings:
        return "General";
    }

    return "Tone";
}

RefHubNavigationState navigationForScenarioView(const std::string_view legacyOrApprovedView)
{
    RefHubNavigationState state;
    state.primary = primaryViewForLegacyView(legacyOrApprovedView);

    const auto& secondary = secondaryRefHubViewNames(state.primary);
    if (contains(secondary, legacyOrApprovedView)) {
        state.secondary = std::string(legacyOrApprovedView);
        return state;
    }

    if (legacyOrApprovedView == "Compare" || legacyOrApprovedView == "Bands") {
        state.secondary = "Tone";
        return state;
    }

    if (legacyOrApprovedView == "Timing") {
        state.secondary = "General";
        return state;
    }

    state.secondary = defaultSecondaryView(state.primary);
    return state;
}

} // namespace ehvota::preview
