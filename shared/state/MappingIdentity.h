#pragma once

#include "ReferenceModel.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ehvota::state {

// Saved mappings trust an Ehvota-owned persistent RefTap UUID, not LiveAPI.id. The Max
// JavaScript LiveAPI id is runtime-dynamic and is retained only as optional diagnostic context.
struct SavedComponentBinding {
    ReferenceRole role { ReferenceRole::other };
    std::string persistentRefTapId;
    std::string lastKnownTrackName;
    std::string lastKnownEndpointName;
    std::string lastKnownLiveRuntimeId;
};

struct MappingIdentityCandidate {
    std::string persistentRefTapId;
    std::string trackName;
    std::string endpointToken; // opaque current RefLink slot:generation token
    bool endpointFresh {};
};

enum class MappingRecoveryState {
    unresolved,
    exactPersistentEndpoint,
    relinkSuggestedByName,
    ambiguousPersistentEndpoint,
    ambiguousNameHint,
    endpointUnavailable
};

struct MappingRecovery {
    MappingRecoveryState state { MappingRecoveryState::unresolved };
    std::optional<MappingIdentityCandidate> candidate;
    std::vector<MappingIdentityCandidate> alternatives;
    bool mayAutoRestore {};
    std::string explanation;
};

[[nodiscard]] MappingRecovery resolveSavedComponentBinding(
    const SavedComponentBinding& saved,
    const std::vector<MappingIdentityCandidate>& candidates);

[[nodiscard]] std::string_view mappingRecoveryStateName(MappingRecoveryState state) noexcept;

} // namespace ehvota::state
