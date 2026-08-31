#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ehvota::communication {

inline constexpr std::uint32_t bridgeProtocolMajor = 1;
inline constexpr std::uint32_t bridgeProtocolMinor = 0;

enum class BridgeCapability : std::uint32_t {
    transportRead = 1u << 0,
    songRead = 1u << 1,
    trackRead = 1u << 2,
    deviceRead = 1u << 3,
    mixerRead = 1u << 4,
    selectionRead = 1u << 5,
    observerEvents = 1u << 6,
    safeMixerWrite = 1u << 7,
    proposalInbox = 1u << 8
};

using BridgeCapabilities = std::uint32_t;

constexpr BridgeCapabilities capabilityBit(const BridgeCapability capability) noexcept
{
    return static_cast<BridgeCapabilities>(capability);
}

constexpr bool hasCapability(const BridgeCapabilities capabilities, const BridgeCapability capability) noexcept
{
    return (capabilities & capabilityBit(capability)) != 0u;
}

enum class BridgeHealth {
    unavailable,
    connecting,
    healthy,
    stale,
    incompatible,
    permissionLimited
};

enum class SourceKind {
    master,
    audioTrack,
    midiTrack,
    returnTrack,
    groupTrack,
    unknown
};

struct ProtocolVersion {
    std::uint32_t major { bridgeProtocolMajor };
    std::uint32_t minor { bridgeProtocolMinor };
};

struct BridgeHello {
    ProtocolVersion protocol {};
    std::string bridgeVersion;
    std::string liveVersion;
    std::string maxVersion;
    BridgeCapabilities capabilities {};
    std::string setIdentity;
};

struct TrackIdentity {
    std::string bridgeId;
    std::string livePath;
    std::string name;
    SourceKind kind { SourceKind::unknown };
    std::optional<std::string> parentBridgeId;
};

struct DeviceIdentity {
    std::string bridgeId;
    std::string livePath;
    std::string trackBridgeId;
    std::string name;
    std::string className;
};

struct Heartbeat {
    std::uint64_t sequence {};
    std::uint64_t monotonicMilliseconds {};
    std::string setIdentity;
};

struct BridgeSnapshot {
    BridgeHello hello;
    Heartbeat heartbeat;
    std::vector<TrackIdentity> tracks;
    std::vector<DeviceIdentity> devices;
};

struct VersionCompatibility {
    bool compatible {};
    std::string reason;
};

VersionCompatibility evaluateCompatibility(const ProtocolVersion& local, const ProtocolVersion& remote);
BridgeHealth evaluateHealth(
    const bool transportConnected,
    const VersionCompatibility& compatibility,
    const std::uint64_t nowMonotonicMilliseconds,
    const Heartbeat& latestHeartbeat,
    const std::uint64_t staleAfterMilliseconds,
    const bool permissionsSufficient);

bool isReadSafe(const BridgeHealth health) noexcept;
bool isWriteSafe(const BridgeHealth health) noexcept;
std::string_view bridgeHealthName(BridgeHealth health) noexcept;

} // namespace ehvota::communication
