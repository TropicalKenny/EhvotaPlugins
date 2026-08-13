#include "shared/safety/ProposalPolicy.h"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

void require(const bool condition, const std::string_view message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

} // namespace

int main()
{
    using namespace ehvota::safety;

    Proposal proposal;
    proposal.id = "proposal-1";
    proposal.state = ProposalState::approved;
    proposal.evidenceId = "capture-chorus-1";
    proposal.userExplicitlyApproved = true;
    proposal.bridgeFresh = true;
    proposal.targetStateFresh = true;
    proposal.undoAvailable = true;
    proposal.changes.push_back({
        "refeq-drum-bus",
        TargetKind::refEq,
        "band1.gainDb",
        0.0,
        -1.2,
        -6.0,
        6.0
    });

    require(validateProposal(proposal).valid, "valid bounded proposal should validate");
    require(mayApply(proposal), "approved explicitly reviewed proposal should be applicable");

    auto stale = proposal;
    stale.targetStateFresh = false;
    require(!validateProposal(stale).valid, "stale target must invalidate proposal");
    require(!mayApply(stale), "stale target must prevent apply");

    auto unapproved = proposal;
    unapproved.userExplicitlyApproved = false;
    require(!mayApply(unapproved), "proposal must never apply without explicit approval");

    auto outOfBounds = proposal;
    outOfBounds.changes.front().proposedValue = -12.0;
    require(!validateProposal(outOfBounds).valid, "out-of-bounds proposal must fail validation");
    require(!mayApply(outOfBounds), "out-of-bounds proposal must not apply");

    auto noUndo = proposal;
    noUndo.undoAvailable = false;
    require(!validateProposal(noUndo).valid, "proposal without undo should fail validation");
    require(!mayApply(noUndo), "proposal without undo must not apply");

    std::cout << "Proposal policy tests passed.\n";
    return EXIT_SUCCESS;
}
