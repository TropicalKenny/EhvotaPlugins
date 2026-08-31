#include "shared/state/MappingValidation.h"

#include <cstdlib>
#include <iostream>
#include <vector>

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
    using namespace ehvota::state;

    ReferencePackage reference;
    reference.sources = {
        { "mix", "Mix", "full_mix.wav", ReferenceRole::fullMix, 180.0, 48000.0, 2, true, false },
        { "drums", "Drums", "drums.wav", ReferenceRole::drums, 180.0, 48000.0, 2, true, true },
        { "bass", "Bass", "bass.wav", ReferenceRole::bass, 180.0, 48000.0, 2, true, true }
    };
    std::vector<LiveSource> live {
        { "drum-bus-id", "DRUM BUS", "RefTap", true },
        { "bass-bus-id", "BASS BUS", "RefTap", true }
    };
    std::vector<MappingEntry> mappings {
        { ReferenceRole::drums, "drums", { "drum-bus-id" }, true },
        { ReferenceRole::bass, "bass", { "bass-bus-id" }, true }
    };

    require(validateMappings(reference, live, mappings).valid,
        "matching reference roles and durable live ids should validate");

    live[0].displayName = "PERCUSSION / DRUMS";
    require(validateMappings(reference, live, mappings).valid,
        "renaming a visible Ableton track must not invalidate an id-based mapping");

    auto disconnected = live;
    disconnected[0].connected = false;
    const auto disconnectedResult = validateMappings(reference, disconnected, mappings);
    require(disconnectedResult.valid,
        "temporarily disconnected live sources should preserve the mapping for recovery");
    require(!disconnectedResult.issues.empty(),
        "disconnected saved mapping should still surface a non-blocking issue");

    auto wrongRole = mappings;
    wrongRole[0].referenceSourceId = "bass";
    require(!validateMappings(reference, live, wrongRole).valid,
        "a Drums mapping pointing at the Bass reference must be rejected");

    auto removedLive = live;
    removedLive.erase(removedLive.begin());
    require(!validateMappings(reference, removedLive, mappings).valid,
        "a removed durable live source must become a blocking mapping issue until remapped");

    std::cout << "Mapping validation tests passed\n";
    return 0;
}
