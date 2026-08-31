#include "shared/safety/LinkProposalAdapter.h"

#include <cassert>

int main()
{
    using namespace ehvota;
    communication::SharedInstanceId target { 3, 9 };
    safety::Proposal proposal;
    proposal.id = "proposal-1";
    proposal.state = safety::ProposalState::approved;
    proposal.evidenceId = "capture-1";
    proposal.userExplicitlyApproved = true;
    proposal.targetStateFresh = true;
    proposal.undoAvailable = true;
    proposal.changes.push_back({
        safety::sharedInstanceTargetId(target), safety::TargetKind::refEq,
        "b2Gain", 0.0, -1.5, -18.0, 18.0
    });

    auto encoded = safety::encodeApprovedSharedProposal(
        proposal, target, communication::SharedPluginKind::refEQ);
    assert(encoded);
    assert(encoded->operationCount == 1);
    assert(encoded->operations[0].expectedCurrentValue == 0.0f);
    assert(encoded->operations[0].value == -1.5f);

    proposal.userExplicitlyApproved = false;
    assert(!safety::encodeApprovedSharedProposal(
        proposal, target, communication::SharedPluginKind::refEQ));

    proposal.userExplicitlyApproved = true;
    proposal.changes[0].targetInstanceId = "3:8";
    assert(!safety::encodeApprovedSharedProposal(
        proposal, target, communication::SharedPluginKind::refEQ));
    return 0;
}
