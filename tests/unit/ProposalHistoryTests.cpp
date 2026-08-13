#include "shared/safety/ProposalHistory.h"

#include <cassert>
#include <cmath>

int main()
{
    using namespace ehvota;

    safety::Proposal original;
    original.id = "eq:chorus:1";
    original.origin = safety::ProposalOrigin::localRefHub;
    original.state = safety::ProposalState::applied;
    original.evidenceId = "chorus-capture-1";
    original.userExplicitlyApproved = true;
    original.bridgeFresh = true;
    original.targetStateFresh = true;
    original.undoAvailable = true;
    original.changes.push_back({
        "4:7", safety::TargetKind::refEq, "b2Gain",
        1.0, -0.5, -18.0, 18.0
    });

    safety::ProposalHistory history(2);
    const communication::SharedInstanceId target { 4, 7 };
    assert(history.recordApplied(original, target));
    assert(!history.recordApplied(original, target));
    assert(history.size() == 1);

    const auto undo = history.makeApprovedUndoForLatest();
    assert(undo.has_value());
    assert(undo->target.slot == 4 && undo->target.generation == 7);
    assert(undo->proposal.state == safety::ProposalState::approved);
    assert(undo->proposal.userExplicitlyApproved);
    assert(safety::mayApply(undo->proposal));
    assert(undo->proposal.changes.size() == 1);
    assert(std::abs(undo->proposal.changes[0].currentValue + 0.5) < 1.0e-9);
    assert(std::abs(undo->proposal.changes[0].proposedValue - 1.0) < 1.0e-9);

    auto second = original;
    second.id = "eq:verse:2";
    assert(history.recordApplied(second, target));
    auto third = original;
    third.id = "eq:bridge:3";
    assert(history.recordApplied(third, target));
    assert(history.size() == 2); // bounded history evicts oldest.
    assert(history.latestApplied()->proposal.id == "eq:bridge:3");

    third.undoAvailable = false;
    safety::ProposalHistory noUndo;
    assert(noUndo.recordApplied(third, target));
    assert(!noUndo.makeApprovedUndoForLatest());
    return 0;
}
