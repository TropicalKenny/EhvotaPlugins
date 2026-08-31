#include "MappingIdentity.h"

namespace ehvota::state {
namespace {

std::vector<MappingIdentityCandidate> matchingPersistentId(
    const std::string& persistentId,
    const std::vector<MappingIdentityCandidate>& candidates)
{
    std::vector<MappingIdentityCandidate> matches;
    if (persistentId.empty()) return matches;
    for (const auto& candidate : candidates)
        if (candidate.persistentRefTapId == persistentId)
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

    if (!saved.persistentRefTapId.empty()) {
        auto exactMatches = matchingPersistentId(saved.persistentRefTapId, candidates);
        if (exactMatches.size() > 1) {
            result.state = MappingRecoveryState::ambiguousPersistentEndpoint;
            result.alternatives = std::move(exactMatches);
            result.explanation = "More than one live RefTap claims the same persistent Ehvota identity; manual repair is required.";
            return result;
        }
        if (exactMatches.size() == 1) {
            result.candidate = exactMatches.front();
            if (!result.candidate->endpointFresh) {
                result.state = MappingRecoveryState::endpointUnavailable;
                result.explanation = "The saved RefTap identity exists, but its current endpoint is stale or unavailable.";
                return result;
            }
            result.state = MappingRecoveryState::exactPersistentEndpoint;
            result.mayAutoRestore = true;
            result.explanation = "Recovered from the persistent RefTap identity saved in the Ableton Set.";
            return result;
        }

        auto nameHints = matchingNameHint(saved.lastKnownTrackName, candidates);
        if (nameHints.size() == 1) {
            result.state = MappingRecoveryState::relinkSuggestedByName;
            result.candidate = nameHints.front();
            result.explanation = "The saved RefTap identity is missing. One track name matches as a relink hint, but automatic restore is forbidden.";
        } else if (nameHints.size() > 1) {
            result.state = MappingRecoveryState::ambiguousNameHint;
            result.alternatives = std::move(nameHints);
            result.explanation = "The saved RefTap identity is missing and multiple tracks share the old name; manual relink is required.";
        } else {
            result.state = MappingRecoveryState::unresolved;
            result.explanation = "The saved RefTap identity is not currently present.";
        }
        return result;
    }

    auto nameHints = matchingNameHint(saved.lastKnownTrackName, candidates);
    if (nameHints.size() == 1) {
        result.state = MappingRecoveryState::relinkSuggestedByName;
        result.candidate = nameHints.front();
        result.explanation = "One track name matches this legacy binding. Confirm it manually to establish a persistent RefTap identity.";
    } else if (nameHints.size() > 1) {
        result.state = MappingRecoveryState::ambiguousNameHint;
        result.alternatives = std::move(nameHints);
        result.explanation = "Multiple tracks match the legacy name hint; no automatic choice is safe.";
    } else {
        result.state = MappingRecoveryState::unresolved;
        result.explanation = "No current RefTap matches this saved mapping.";
    }
    return result;
}

std::string_view mappingRecoveryStateName(const MappingRecoveryState state) noexcept
{
    switch (state) {
    case MappingRecoveryState::unresolved: return "Unresolved";
    case MappingRecoveryState::exactPersistentEndpoint: return "Exact persistent endpoint";
    case MappingRecoveryState::relinkSuggestedByName: return "Relink suggested";
    case MappingRecoveryState::ambiguousPersistentEndpoint: return "Duplicate persistent endpoint";
    case MappingRecoveryState::ambiguousNameHint: return "Ambiguous name hint";
    case MappingRecoveryState::endpointUnavailable: return "Endpoint unavailable";
    }
    return "Unresolved";
}

} // namespace ehvota::state
