#include "shared/communication/BridgeProtocol.h"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

void require(const bool condition, const std::string_view message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

} // namespace

int main()
{
    using namespace ehvota::communication;

    const ProtocolVersion local { 1, 0 };
    require(evaluateCompatibility(local, { 1, 0 }).compatible, "matching protocol should be compatible");
    require(evaluateCompatibility(local, { 1, 2 }).compatible, "newer remote minor should be compatible");
    require(!evaluateCompatibility(local, { 2, 0 }).compatible, "different major must be incompatible");

    Heartbeat heartbeat { 42, 10'000, "set-1" };
    const auto compatible = evaluateCompatibility(local, { 1, 0 });

    require(
        evaluateHealth(true, compatible, 10'250, heartbeat, 1'000, true) == BridgeHealth::healthy,
        "fresh heartbeat with permissions should be healthy");
    require(
        evaluateHealth(true, compatible, 11'500, heartbeat, 1'000, true) == BridgeHealth::stale,
        "old heartbeat should be stale");
    require(
        evaluateHealth(false, compatible, 10'250, heartbeat, 1'000, true) == BridgeHealth::unavailable,
        "disconnected transport should be unavailable");
    require(
        evaluateHealth(true, compatible, 10'250, heartbeat, 1'000, false) == BridgeHealth::permissionLimited,
        "insufficient permissions should be explicit");

    require(isReadSafe(BridgeHealth::permissionLimited), "permission-limited bridge may remain read safe");
    require(!isWriteSafe(BridgeHealth::permissionLimited), "permission-limited bridge must reject writes");
    require(!isReadSafe(BridgeHealth::stale), "stale bridge must not be trusted for fresh reads");
    require(!isWriteSafe(BridgeHealth::stale), "stale bridge must reject writes");

    const auto caps = capabilityBit(BridgeCapability::songRead)
        | capabilityBit(BridgeCapability::trackRead)
        | capabilityBit(BridgeCapability::observerEvents);
    require(hasCapability(caps, BridgeCapability::songRead), "songRead capability missing");
    require(!hasCapability(caps, BridgeCapability::safeMixerWrite), "unexpected write capability");

    std::cout << "Bridge protocol tests passed.\n";
    return EXIT_SUCCESS;
}
