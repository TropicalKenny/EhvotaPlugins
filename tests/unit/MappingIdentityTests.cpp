#include "shared/state/MappingIdentity.h"

#include <cassert>
#include <vector>

int main()
{
    using namespace ehvota::state;

    SavedComponentBinding saved;
    saved.role = ReferenceRole::drums;
    saved.persistentRefTapId = "44b6f42c-38aa-4a3a-b5f0-1f4d6d270001";
    saved.lastKnownTrackName = "DRUM BUS";

    std::vector<MappingIdentityCandidate> candidates {
        { "44b6f42c-38aa-4a3a-b5f0-1f4d6d270001", "DRUM BUS RENAMED", "refTap:2:8", true },
        { "44b6f42c-38aa-4a3a-b5f0-1f4d6d270002", "BASS BUS", "refTap:4:3", true }
    };
    auto exact = resolveSavedComponentBinding(saved, candidates);
    assert(exact.state == MappingRecoveryState::exactPersistentEndpoint);
    assert(exact.mayAutoRestore);
    assert(exact.candidate.has_value());
    assert(exact.candidate->trackName == "DRUM BUS RENAMED");

    candidates[0].endpointFresh = false;
    auto unavailable = resolveSavedComponentBinding(saved, candidates);
    assert(unavailable.state == MappingRecoveryState::endpointUnavailable);
    assert(!unavailable.mayAutoRestore);

    candidates = { { "new-ref-tap", "DRUM BUS", "refTap:1:1", true } };
    auto hint = resolveSavedComponentBinding(saved, candidates);
    assert(hint.state == MappingRecoveryState::relinkSuggestedByName);
    assert(hint.candidate.has_value());
    assert(!hint.mayAutoRestore);

    candidates.push_back({ "another-ref-tap", "DRUM BUS", "refTap:2:1", true });
    auto ambiguousName = resolveSavedComponentBinding(saved, candidates);
    assert(ambiguousName.state == MappingRecoveryState::ambiguousNameHint);
    assert(ambiguousName.alternatives.size() == 2);

    candidates = {
        { saved.persistentRefTapId, "DRUM BUS A", "refTap:1:1", true },
        { saved.persistentRefTapId, "DRUM BUS B", "refTap:2:1", true }
    };
    auto duplicateId = resolveSavedComponentBinding(saved, candidates);
    assert(duplicateId.state == MappingRecoveryState::ambiguousPersistentEndpoint);
    assert(!duplicateId.mayAutoRestore);

    SavedComponentBinding legacy;
    legacy.role = ReferenceRole::vocals;
    legacy.lastKnownTrackName = "VOCAL BUS";
    candidates = { { "persistent-vocal", "VOCAL BUS", "refTap:9:1", true } };
    auto legacyHint = resolveSavedComponentBinding(legacy, candidates);
    assert(legacyHint.state == MappingRecoveryState::relinkSuggestedByName);
    assert(!legacyHint.mayAutoRestore);

    return 0;
}
