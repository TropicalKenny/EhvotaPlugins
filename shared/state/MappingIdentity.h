#pragma once

#include "ReferenceModel.h"

#include <optional>
#include <string>
#include <vector>

namespace ehvota::state {

// Durable mapping identity is deliberately separate from RefLink slot/generation, which is
// only a runtime process-local endpoint address. bridgeTrackId comes from Ehvota Bridge.
struct SavedComponentBinding {
    ReferenceRole role { ReferenceRole::other };
    std::string bridgeTrackId;
    std::string lastKnownTrackName;
    std::string lastKnownEndpointName;
};

struct MappingIdentityCandidate {
    std::string bridgeTrackId;
    std::string trackName;
    std::string endpointToken; // opaque RefHub runtime token; not persisted as identity
    bool endpointFresh {};
};

enum class MappingRecoveryState {
    unresolved,
    exactBridgeIdentity,
    relinkSuggestedByName,
    ambiguousBridgeIdentity,
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
