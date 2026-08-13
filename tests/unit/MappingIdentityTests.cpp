#include "shared/state/MappingIdentity.h"

#include <cassert>
#include <vector>

int main()
{
    using namespace ehvota::state;

    SavedComponentBinding saved;
    saved.role = ReferenceRole::drums;
    saved.bridgeTrackId = "track:drums:stable";
    saved.lastKnownTrackName = "DRUM BUS";

    std::vector<MappingIdentityCandidate> candidates {
        { "track:drums:stable", "DRUM BUS RENAMED", "refTap:2:8", true },
        { "track:bass:stable", "BASS BUS", "refTap:4:3", true }
    };
    auto exact = resolveSavedComponentBinding(saved, candidates);
    assert(exact.state == MappingRecoveryState::exactBridgeIdentity);
    assert(exact.mayAutoRestore);
    assert(exact.candidate.has_value());
    assert(exact.candidate->trackName == "DRUM BUS RENAMED");

    candidates[0].endpointFresh = false;
    auto unavailable = resolveSavedComponentBinding(saved, candidates);
    assert(unavailable.state == MappingRecoveryState::endpointUnavailable);
    assert(!unavailable.mayAutoRestore);

    candidates = {
        { "track:new-one", "DRUM BUS", "refTap:1:1", true }
    };
    auto hint = resolveSavedComponentBinding(saved, candidates);
    assert(hint.state == MappingRecoveryState::relinkSuggestedByName);
    assert(hint.candidate.has_value());
    assert(!hint.mayAutoRestore);

    candidates.push_back({ "track:new-two", "DRUM BUS", "refTap:2:1", true });
    auto ambiguousName = resolveSavedComponentBinding(saved, candidates);
    assert(ambiguousName.state == MappingRecoveryState::ambiguousNameHint);
    assert(ambiguousName.alternatives.size() == 2);
    assert(!ambiguousName.mayAutoRestore);

    candidates = {
        { "track:drums:stable", "DRUM BUS A", "refTap:1:1", true },
        { "track:drums:stable", "DRUM BUS B", "refTap:2:1", true }
    };
    auto badIdentity = resolveSavedComponentBinding(saved, candidates);
    assert(badIdentity.state == MappingRecoveryState::ambiguousBridgeIdentity);
    assert(!badIdentity.mayAutoRestore);

    SavedComponentBinding legacy;
    legacy.role = ReferenceRole::vocals;
    legacy.lastKnownTrackName = "VOCAL BUS";
    candidates = { { "track:vocal", "VOCAL BUS", "refTap:9:1", true } };
    auto legacyHint = resolveSavedComponentBinding(legacy, candidates);
    assert(legacyHint.state == MappingRecoveryState::relinkSuggestedByName);
    assert(!legacyHint.mayAutoRestore);

    return 0;
}
