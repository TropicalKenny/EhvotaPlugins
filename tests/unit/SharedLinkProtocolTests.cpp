#include "shared/communication/SharedLinkProtocol.h"
#include <cassert>
#include <limits>

int main()
{
    using namespace ehvota::communication;
    static_assert(sharedLinkVersion == 3U);
    assert(sharedParameterAllowed(SharedPluginKind::refEQ, SharedParameterKey::eqMode));
    assert(sharedParameterAllowed(SharedPluginKind::refEQ, SharedParameterKey::eqBand3Gain));
    assert(!sharedParameterAllowed(SharedPluginKind::refTap, SharedParameterKey::eqBand3Gain));
    assert(validateSharedOperation(SharedPluginKind::refEQ, {SharedParameterKey::eqMode,0.0f,0.001f,2.0f}));
    assert(!validateSharedOperation(SharedPluginKind::refEQ, {SharedParameterKey::eqMode,0.0f,0.001f,1.5f}));
    assert(validateSharedOperation(SharedPluginKind::refEQ,{SharedParameterKey::eqBand1Gain,0.0f,0.01f,-2.5f}));
    assert(!validateSharedOperation(SharedPluginKind::refEQ,{SharedParameterKey::eqBand1Gain,0.0f,0.01f,-40.0f}));
    assert(!validateSharedOperation(SharedPluginKind::refEQ,{SharedParameterKey::eqBand1Gain,0.0f,0.01f,std::numeric_limits<float>::quiet_NaN()}));
    assert(!validateSharedOperation(SharedPluginKind::refComp,{SharedParameterKey::compMode,0.0f,0.001f,1.5f}));
    SharedProposalMailbox proposal;
    proposal.proposalId=42;
    proposal.targetGeneration=7;
    proposal.operationCount=2;
    proposal.operations[0]={SharedParameterKey::compThreshold,-18.0f,0.001f,-16.0f};
    proposal.operations[1]={SharedParameterKey::compRatio,4.0f,0.001f,2.5f};
    assert(validateSharedProposal(SharedPluginKind::refComp,proposal));
    proposal.operations[1]={SharedParameterKey::eqBand1Gain,0.0f,0.001f,1.0f};
    assert(!validateSharedProposal(SharedPluginKind::refComp,proposal));
    static_assert(sizeof(SharedLinkRegion)<1024*1024);
    return 0;
}
