#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ehvota::preview {

enum class AbletonBridgeHealth {
    unavailable,
    connecting,
    healthy,
    stale,
    incompatible
};

enum class ExternalControlPolicy {
    disabled,
    inspectOnly,
    proposeOnly,
    approvedWrites
};

struct AbletonTrackEndpoint {
    std::string stableKey;
    std::string currentName;
    std::string mappedName;
    std::string semanticRole;
    bool exists { true };
    bool connected { true };
    bool renamedSinceMapping { false };
};

struct AbletonBridgeState {
    std::string id;
    std::string title;
    std::string summary;
    AbletonBridgeHealth health { AbletonBridgeHealth::unavailable };
    ExternalControlPolicy controlPolicy { ExternalControlPolicy::disabled };
    std::string liveSetName;
    std::string liveVersion;
    std::string bridgeVersion;
    std::uint64_t heartbeatAgeMs { 0 };
    bool maxForLiveAvailable { false };
    bool mcpAvailable { false };
    bool audioProcessingSafe { true };
    bool externalProposalPending { false };
    bool externalWriteDenied { false };
    std::vector<AbletonTrackEndpoint> tracks;
    std::vector<std::string> notices;
};

[[nodiscard]] const std::vector<AbletonBridgeState>& allAbletonBridgeStates();
[[nodiscard]] const AbletonBridgeState* findAbletonBridgeState(std::string_view id);
[[nodiscard]] std::vector<std::string> validateAbletonBridgeStates();
[[nodiscard]] bool bridgeCanInspect(const AbletonBridgeState& state) noexcept;
[[nodiscard]] bool bridgeCanCreateProposal(const AbletonBridgeState& state) noexcept;
[[nodiscard]] bool bridgeCanApplyApprovedWrite(const AbletonBridgeState& state) noexcept;

} // namespace ehvota::preview
