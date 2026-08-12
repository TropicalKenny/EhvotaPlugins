#include "shared/viewmodels/RefHubNavigation.h"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

using ehvota::preview::RefHubPrimaryView;
using ehvota::preview::navigationForScenarioView;
using ehvota::preview::primaryRefHubViewNames;
using ehvota::preview::secondaryRefHubViewNames;

void require(const bool condition, const std::string_view message)
{
    if (condition)
        return;

    std::cerr << "RefHubNavigationTests: " << message << '\n';
    std::exit(EXIT_FAILURE);
}

void testExactlyFivePrimaryDestinations()
{
    const auto& views = primaryRefHubViewNames();
    require(views.size() == 5, "RefHub must expose exactly five primary destinations");
    require(views[0] == "Compare", "Compare must be first");
    require(views[1] == "Sources", "Sources must be second");
    require(views[2] == "Process", "Process must be third");
    require(views[3] == "History", "History must be fourth");
    require(views[4] == "Settings", "Settings must be fifth");
}

void testLegacyViewsCollapseIntoApprovedStructure()
{
    auto state = navigationForScenarioView("Bands");
    require(state.primary == RefHubPrimaryView::compare, "Bands must no longer be a primary destination");
    require(state.secondary == "Tone", "Bands must collapse into the Compare/Tone lens");

    state = navigationForScenarioView("Mapping");
    require(state.primary == RefHubPrimaryView::sources, "Mapping belongs under Sources");
    require(state.secondary == "Mapping", "Mapping should remain an explicit Sources secondary view");

    state = navigationForScenarioView("Proposals");
    require(state.primary == RefHubPrimaryView::process, "Proposals belong under Process");
    require(state.secondary == "Proposals", "Proposals should remain an explicit Process secondary view");

    state = navigationForScenarioView("Timing");
    require(state.primary == RefHubPrimaryView::settings, "legacy RefHub Timing must not remain a primary destination");
}

void testCompareLenses()
{
    const auto& compare = secondaryRefHubViewNames(RefHubPrimaryView::compare);
    require(compare.size() == 5, "Compare must expose five approved lenses");
    require(compare[0] == "Overview", "Compare lens 1 should be Overview");
    require(compare[1] == "Tone", "Compare lens 2 should be Tone");
    require(compare[2] == "Dynamics", "Compare lens 3 should be Dynamics");
    require(compare[3] == "Space", "Compare lens 4 should be Space");
    require(compare[4] == "Components", "Compare lens 5 should be Components");
}

} // namespace

int main()
{
    testExactlyFivePrimaryDestinations();
    testLegacyViewsCollapseIntoApprovedStructure();
    testCompareLenses();
    return EXIT_SUCCESS;
}
