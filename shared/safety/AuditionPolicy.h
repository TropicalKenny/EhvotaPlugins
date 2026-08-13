#pragma once

namespace ehvota::safety {

struct AuditionPolicyInput {
    bool referenceRequested {};
    bool referenceLoaded {};
    bool renderSafe { true };
    bool hostNonRealtime {};
};

struct AuditionPolicyDecision {
    bool playReference {};
    bool preserveProjectSignal { true };
    bool suppressedByRenderSafe {};
};

[[nodiscard]] AuditionPolicyDecision decideReferenceAudition(const AuditionPolicyInput& input) noexcept;

} // namespace ehvota::safety
