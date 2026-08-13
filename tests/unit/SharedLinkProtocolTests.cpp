#include "shared/communication/SharedLinkProtocol.h"

#include <cassert>
#include <cmath>
#include <limits>

int main()
{
    using namespace ehvota::communication;

    assert(sharedParameterAllowed(SharedPluginKind::refEQ, SharedParameterKey::eqBand3Gain));
    assert(!sharedParameterAllowed(SharedPluginKind::refTap, SharedParameterKey::eqBand3Gain));
    assert(sharedParameterAllowed(SharedPluginKind::refComp, SharedParameterKey::compAttackSync));
    assert(sharedParameterAllowed(SharedPluginKind::refColor, SharedParameterKey::colorAmount));
    assert(sharedParameterAllowed(SharedPluginKind::refTransient, SharedParameterKey::transientAttack));

    assert(validateSharedOperation(
        SharedPluginKind::refEQ,
        { SharedParameterKey::eqBand1Gain, -2.5f }));
    assert(!validateSharedOperation(
        SharedPluginKind::refEQ,
        { SharedParameterKey::eqBand1Gain, -40.0f }));
    assert(!validateSharedOperation(
        SharedPluginKind::refEQ,
        { SharedParameterKey::eqBand1Gain, std::numeric_limits<float>::quiet_NaN() }));
    assert(!validateSharedOperation(
        SharedPluginKind::refComp,
        { SharedParameterKey::compMode, 1.5f }));

    SharedProposalMailbox proposal;
    proposal.proposalId = 42;
    proposal.targetGeneration = 7;
    proposal.operationCount = 2;
    proposal.operations[0] = { SharedParameterKey::compThreshold, -16.0f };
    proposal.operations[1] = { SharedParameterKey::compRatio, 2.5f };
    assert(validateSharedProposal(SharedPluginKind::refComp, proposal));

    proposal.operations[1] = { SharedParameterKey::eqBand1Gain, 1.0f };
    assert(!validateSharedProposal(SharedPluginKind::refComp, proposal));

    proposal.operationCount = static_cast<std::uint32_t>(sharedLinkMaxOperations + 1);
    assert(!validateSharedProposal(SharedPluginKind::refComp, proposal));

    static_assert(sizeof(SharedLinkRegion) < 1024 * 1024);
    return 0;
}
