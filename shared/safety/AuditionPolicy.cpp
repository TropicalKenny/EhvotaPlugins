#include "AuditionPolicy.h"

namespace ehvota::safety {

AuditionPolicyDecision decideReferenceAudition(const AuditionPolicyInput& input) noexcept
{
    AuditionPolicyDecision decision;
    decision.suppressedByRenderSafe = input.referenceRequested
        && input.referenceLoaded
        && input.renderSafe
        && input.hostNonRealtime;

    decision.playReference = input.referenceRequested
        && input.referenceLoaded
        && !decision.suppressedByRenderSafe;
    decision.preserveProjectSignal = !decision.playReference;
    return decision;
}

} // namespace ehvota::safety
