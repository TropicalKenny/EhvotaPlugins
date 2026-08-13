#include "MappingIdentity.h"

#include <algorithm>

namespace ehvota::state {
namespace {

std::vector<MappingIdentityCandidate> matchingBridgeId(
    const std::string& bridgeId,
    const std::vector<MappingIdentityCandidate>& candidates)
{
    std::vector<MappingIdentityCandidate> matches;
    if (bridgeId.empty()) return matches;
    for (const auto& candidate : candidates)
        if (candidate.bridgeTrackId == bridgeId)
            matches.push_back(candidate);
    return matches;
}

std::vector<MappingIdentityCandidate> matchingNameHint(
    const std::string& name,
    const std::vector<MappingIdentityCandidate>& candidates)
{
    std::vector<MappingIdentityCandidate> matches;
    if (name.empty()) return matches;
    for (const auto& candidate : candidates)
        if (candidate.trackName == name)
            matches.push_back(candidate);
    return matches;
}

} // namespace

MappingRecovery resolveSavedComponentBinding(
    const SavedComponentBinding& saved,
    const std::vector<MappingIdentityCandidate>& candidates)
{
    MappingRecovery result;

    if (!saved.bridgeTrackId.empty()) {
        auto bridgeMatches = matchingBridgeId(saved.bridgeTrackId, candidates);
        if (bridgeMatches.size() > 1) {
            result.state = MappingRecoveryState::ambiguousBridgeIdentity;
            result.alternatives = std::move(bridgeMatches);
            result.explanation = "More than one runtime endpoint claims the saved Ableton bridge track identity; manual relink is required.";
            return result;
        }
        if (bridgeMatches.size() == 1) {
            result.candidate = bridgeMatches.front();
            if (!result.candidate->endpointFresh) {
                result.state = MappingRecoveryState::endpointUnavailable;
                result.explanation = "The saved Ableton track identity exists, but its mapped Ehvota endpoint is not currently fresh.";
                return result;
            }
            result.state = MappingRecoveryState::exactBridgeIdentity;
            result.mayAutoRestore = true;
            result.explanation = "Recovered from the saved Ableton bridge track identity.";
            return result;
        }

        // A name match after an identity miss is only a hint. A renamed/recreated track must
        // never inherit processing merely because its visible name happens to match.
        auto nameHints = matchingNameHint(saved.lastKnownTrackName, candidates);
        if (nameHints.size() == 1) {
            result.state = MappingRecoveryState::relinkSuggestedByName;
            result.candidate = nameHints.front();
            result.explanation = "The saved bridge identity is missing. One track name matches as a relink hint, but automatic restore is forbidden.";
        } else if (nameHints.size() > 1) {
            result.state = MappingRecoveryState::ambiguousNameHint;
            result.alternatives = std::move(nameHints);
            result.explanation = "The saved bridge identity is missing and multiple tracks share the previous name; manual relink is required.";
        } else {
            result.state = MappingRecoveryState::unresolved;
            result.explanation = "The saved Ableton track identity is no longer present.";
        }
        return result;
    }

    // Legacy/manual bindings that predate bridge identity can surface a useful suggestion,
    // but visible names are never sufficient proof for an automatic mapping.
    auto nameHints = matchingNameHint(saved.lastKnownTrackName, candidates);
    if (nameHints.size() == 1) {
        result.state = MappingRecoveryState::relinkSuggestedByName;
        result.candidate = nameHints.front();
        result.explanation = "One track name matches this legacy binding. Confirm it manually to establish a durable bridge identity.";
    } else if (nameHints.size() > 1) {
        result.state = MappingRecoveryState::ambiguousNameHint;
        result.alternatives = std::move(nameHints);
        result.explanation = "Multiple tracks match the legacy name hint; no automatic choice is safe.";
    } else {
        result.state = MappingRecoveryState::unresolved;
        result.explanation = "No current track matches this saved mapping.";
    }
    return result;
}

std::string_view mappingRecoveryStateName(const MappingRecoveryState state) noexcept
{
    switch (state) {
    case MappingRecoveryState::unresolved: return "Unresolved";
    case MappingRecoveryState::exactBridgeIdentity: return "Exact bridge identity";
    case MappingRecoveryState::relinkSuggestedByName: return "Relink suggested";
    case MappingRecoveryState::ambiguousBridgeIdentity: return "Ambiguous bridge identity";
    case MappingRecoveryState::ambiguousNameHint: return "Ambiguous name hint";
    case MappingRecoveryState::endpointUnavailable: return "Endpoint unavailable";
    }
    return "Unresolved";
}

} // namespace ehvota::state
