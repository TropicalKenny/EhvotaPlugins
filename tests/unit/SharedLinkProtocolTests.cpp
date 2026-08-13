#include "shared/communication/SharedLinkProtocol.h"

#include <cassert>
#include <limits>

int main()
{
    using namespace ehvota::communication;

    assert(sharedParameterAllowed(SharedPluginKind::refEQ, SharedParameterKey::eqBand3Gain));
    assert(!sharedParameterAllowed(SharedPluginKind::refTap, SharedParameterKey::eqBand3Gain));
    assert(validateSharedOperation(SharedPluginKind::refEQ,
        { SharedParameterKey::eqBand1Gain, 0.0f, 0.01f, -2.5f }));
    assert(!validateSharedOperation(SharedPluginKind::refEQ,
        { SharedParameterKey::eqBand1Gain, 0.0f, 0.01f, -40.0f }));
    assert(!validateSharedOperation(SharedPluginKind::refEQ,
        { SharedParameterKey::eqBand1Gain, 0.0f, 0.01f, std::numeric_limits<float>::quiet_NaN() }));
    assert(!validateSharedOperation(SharedPluginKind::refComp,
        { SharedParameterKey::compMode, 0.0f, 0.001f, 1.5f }));

    SharedProposalMailbox proposal;
    proposal.proposalId = 42;
    proposal.targetGeneration = 7;
    proposal.operationCount = 2;
    proposal.operations[0] = { SharedParameterKey::compThreshold, -18.0f, 0.001f, -16.0f };
    proposal.operations[1] = { SharedParameterKey::compRatio, 4.0f, 0.001f, 2.5f };
    assert(validateSharedProposal(SharedPluginKind::refComp, proposal));
    proposal.operations[1] = { SharedParameterKey::eqBand1Gain, 0.0f, 0.001f, 1.0f };
    assert(!validateSharedProposal(SharedPluginKind::refComp, proposal));

    SharedCaptureRequest capture;
    capture.captureId = 1001;
    capture.targetGeneration = 4;
    capture.startHostSample = 441000;
    capture.durationSamples = 44100 * 8;
    assert(validateSharedCaptureRequest(capture));
    capture.durationSamples = 0;
    assert(!validateSharedCaptureRequest(capture));
    capture.durationSamples = sharedCaptureMaximumSamples + 1U;
    assert(!validateSharedCaptureRequest(capture));
    capture.durationSamples = 44100;
    capture.startHostSample = -2;
    assert(!validateSharedCaptureRequest(capture));

    SharedCapturedAnalysis analysis;
    analysis.captureId = 1001;
    analysis.sourceGeneration = 4;
    analysis.valid = 1;
    analysis.sampleRate = 44100.0f;
    analysis.capturedSamples = 44100 * 8;
    analysis.integratedLufs = -16.2f;
    analysis.peakDb = -4.0f;
    analysis.rmsDb = -18.0f;
    analysis.crestDb = 14.0f;
    analysis.correlation = 0.72f;
    analysis.sideToMidDb = -8.0f;
    analysis.bandPowerDb = { -22.0f, -20.0f, -24.0f, -26.0f, -30.0f, -36.0f };
    assert(validateSharedCapturedAnalysis(analysis));
    analysis.correlation = 1.5f;
    assert(!validateSharedCapturedAnalysis(analysis));

    assert(sharedCaptureStateInFlight(SharedCaptureState::pending));
    assert(sharedCaptureStateInFlight(SharedCaptureState::analysing));
    assert(!sharedCaptureStateInFlight(SharedCaptureState::complete));

    static_assert(sizeof(SharedLinkRegion) < 1024 * 1024);
    return 0;
}
