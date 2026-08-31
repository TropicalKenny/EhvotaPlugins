#include "BridgeProtocol.h"

namespace ehvota::communication {

VersionCompatibility evaluateCompatibility(const ProtocolVersion& local, const ProtocolVersion& remote)
{
    if (local.major != remote.major)
        return { false, "Protocol major versions differ." };

    if (remote.minor < local.minor)
        return { false, "Remote bridge protocol is older than the required local minor version." };

    return { true, {} };
}

BridgeHealth evaluateHealth(
    const bool transportConnected,
    const VersionCompatibility& compatibility,
    const std::uint64_t nowMonotonicMilliseconds,
    const Heartbeat& latestHeartbeat,
    const std::uint64_t staleAfterMilliseconds,
    const bool permissionsSufficient)
{
    if (!transportConnected)
        return BridgeHealth::unavailable;

    if (!compatibility.compatible)
        return BridgeHealth::incompatible;

    if (nowMonotonicMilliseconds < latestHeartbeat.monotonicMilliseconds)
        return BridgeHealth::stale;

    if ((nowMonotonicMilliseconds - latestHeartbeat.monotonicMilliseconds) > staleAfterMilliseconds)
        return BridgeHealth::stale;

    if (!permissionsSufficient)
        return BridgeHealth::permissionLimited;

    return BridgeHealth::healthy;
}

bool isReadSafe(const BridgeHealth health) noexcept
{
    return health == BridgeHealth::healthy || health == BridgeHealth::permissionLimited;
}

bool isWriteSafe(const BridgeHealth health) noexcept
{
    return health == BridgeHealth::healthy;
}

std::string_view bridgeHealthName(const BridgeHealth health) noexcept
{
    switch (health) {
    case BridgeHealth::unavailable: return "Unavailable";
    case BridgeHealth::connecting: return "Connecting";
    case BridgeHealth::healthy: return "Healthy";
    case BridgeHealth::stale: return "Stale";
    case BridgeHealth::incompatible: return "Incompatible";
    case BridgeHealth::permissionLimited: return "Permission limited";
    }

    return "Unknown";
}

} // namespace ehvota::communication
