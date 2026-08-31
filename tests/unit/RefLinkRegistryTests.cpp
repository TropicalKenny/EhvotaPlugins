#include "shared/communication/RefLinkRegistry.h"

#include <array>
#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}
} // namespace

int main()
{
    auto& registry = ehvota::communication::RefLinkRegistry::instance();
    const auto handle = registry.registerInstance(ehvota::communication::RefLinkRole::drums, "Drum Bus RefTap");
    require(handle.valid(), "RefLink registration should return a valid handle");

    registry.publish(handle, { -1.0f, -12.0f, 0.75f, 11.0f, 4096 });
    const auto snapshot = registry.read(handle);
    require(snapshot.has_value(), "published RefLink instance should be readable");
    require(snapshot->role == ehvota::communication::RefLinkRole::drums, "semantic role should survive registration");
    require(snapshot->metrics.sampleCounter == 4096, "published sample counter should be visible");
    require(snapshot->metrics.correlation == 0.75f, "published correlation should be visible");

    std::array<ehvota::communication::RefLinkInstanceSnapshot, ehvota::communication::RefLinkRegistry::maxInstances> active {};
    const auto activeCount = registry.readActive(active);
    require(activeCount >= 1, "active enumeration should include registered instance");

    registry.unregisterInstance(handle);
    require(!registry.read(handle).has_value(), "unregistered handle must become unreadable");

    registry.publish(handle, { 0.0f, 0.0f, 0.0f, 0.0f, 8192 });
    require(!registry.read(handle).has_value(), "stale handle must not reactivate a released slot");

    std::cout << "RefLink registry tests passed\n";
    return 0;
}
