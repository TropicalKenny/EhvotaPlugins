#include "shared/viewmodels/AbletonBridgeState.h"

#include <iostream>

int main()
{
    using namespace ehvota::preview;

    const auto errors = validateAbletonBridgeStates();
    if (!errors.empty()) {
        for (const auto& error : errors)
            std::cerr << error << '\n';
        return 1;
    }

    const auto* healthy = findAbletonBridgeState("ableton-bridge-healthy");
    const auto* unavailable = findAbletonBridgeState("ableton-bridge-unavailable");
    const auto* stale = findAbletonBridgeState("ableton-bridge-stale");
    const auto* pending = findAbletonBridgeState("external-proposal-pending");
    const auto* denied = findAbletonBridgeState("external-write-denied");

    if (healthy == nullptr || unavailable == nullptr || stale == nullptr || pending == nullptr || denied == nullptr) {
        std::cerr << "Required Ableton bridge fixtures were not found.\n";
        return 1;
    }

    if (!bridgeCanInspect(*healthy)) {
        std::cerr << "Healthy bridge fixture should permit inspection.\n";
        return 1;
    }

    if (bridgeCanInspect(*unavailable) || bridgeCanCreateProposal(*unavailable)) {
        std::cerr << "Unavailable bridge must not permit inspection or proposals.\n";
        return 1;
    }

    if (bridgeCanCreateProposal(*stale)) {
        std::cerr << "Stale bridge must not permit proposal creation.\n";
        return 1;
    }

    if (!bridgeCanCreateProposal(*pending) || bridgeCanApplyApprovedWrite(*pending)) {
        std::cerr << "Pending proposal must allow proposal creation but not direct apply.\n";
        return 1;
    }

    if (bridgeCanApplyApprovedWrite(*denied)) {
        std::cerr << "Denied write must never be applied.\n";
        return 1;
    }

    for (const auto& state : allAbletonBridgeStates()) {
        if (!state.audioProcessingSafe) {
            std::cerr << "Bridge state incorrectly marks core audio processing unsafe: " << state.id << '\n';
            return 1;
        }
    }

    std::cout << "Ableton bridge state tests passed: " << allAbletonBridgeStates().size() << " fixtures.\n";
    return 0;
}
