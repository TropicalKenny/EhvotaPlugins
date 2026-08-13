#include "shared/state/AnalysisScope.h"

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

    auto noVocals = AnalysisScope::all();
    noVocals.setIncluded(ReferenceRole::vocals, false);
    require(noVocals.includes(ReferenceRole::fullMix), "authoritative full mix must remain included regardless of component scope");
    require(!noVocals.includes(ReferenceRole::vocals), "vocals should be explicitly excludable from component diagnostics");
    require(noVocals.includes(ReferenceRole::drums), "excluding vocals must not implicitly exclude drums");

    const auto drumsOnly = AnalysisScope::only(ReferenceRole::drums);
    require(drumsOnly.drums && !drumsOnly.bass && !drumsOnly.vocals && !drumsOnly.music,
        "drums-only mode should select exactly the requested diagnostic component");

    ReferencePackage package;
    package.id = "ref";
    package.name = "Reference";
    package.sources = {
        { "mix", "Mix", "full_mix.wav", ReferenceRole::fullMix, 180.0, 48000.0, 2, true, false },
        { "drums", "Drums", "drums.wav", ReferenceRole::drums, 180.0, 48000.0, 2, true, true },
        { "bass", "Bass", "bass.wav", ReferenceRole::bass, 180.0, 48000.0, 2, true, true },
        { "vocals", "Vocals", "vocals.wav", ReferenceRole::vocals, 180.0, 48000.0, 2, true, true }
    };

    std::vector<LiveSource> live {
        { "drum-bus", "DRUM BUS", "RefTap", true },
        { "bass-bus", "BASS BUS", "RefTap", true },
        { "vocal-bus", "VOCAL BUS", "RefTap", false }
    };
    std::vector<MappingEntry> mappings {
        { ReferenceRole::drums, "drums", { "drum-bus" }, true },
        { ReferenceRole::bass, "bass", { "bass-bus" }, true },
        { ReferenceRole::vocals, "vocals", { "vocal-bus" }, true }
    };

    const auto availability = evaluateScopeAvailability(noVocals, package, mappings, live);
    require(availability[0].usable, "requested drums with a stem and connected mapping should be usable");
    require(availability[1].usable, "requested bass with a stem and connected mapping should be usable");
    require(!availability[2].requested && !availability[2].usable,
        "excluded vocals should remain unused even if a reference stem exists");
    require(availability[3].requested && !availability[3].hasReferenceStem && !availability[3].usable,
        "requested music without a reference stem should be reported unavailable rather than guessed");

    std::cout << "Analysis scope tests passed\n";
    return 0;
}
