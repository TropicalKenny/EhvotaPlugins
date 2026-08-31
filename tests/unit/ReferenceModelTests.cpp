#include "shared/state/ReferenceModel.h"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {
void require(bool condition, std::string_view message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
}

int main()
{
    using namespace ehvota::state;

    ReferencePackage package;
    package.id = "ayo-nights";
    package.name = "Ayo Nights";
    package.sources = {
        { "mix", "Full Mix", "full_mix.wav", ReferenceRole::fullMix, 210.0, 48000.0, 2, true, false },
        { "drums", "Drums", "drums.wav", ReferenceRole::drums, 210.0, 48000.0, 2, true, true },
        { "bass", "Bass", "bass.wav", ReferenceRole::bass, 210.0, 48000.0, 2, true, true },
        { "vocals", "Vocals", "vocals.wav", ReferenceRole::vocals, 210.0, 48000.0, 2, true, true },
        { "music", "Music", "music.wav", ReferenceRole::music, 210.0, 48000.0, 2, true, true }
    };

    require(validateReferencePackage(package).valid, "aligned reference package should validate");
    require(authoritativeFullMix(package).has_value(), "full mix should be discoverable");
    require(!isDiagnosticRole(ReferenceRole::fullMix), "full mix must remain authoritative");
    require(isDiagnosticRole(ReferenceRole::drums), "stems are diagnostic");

    auto missingMix = package;
    missingMix.sources.erase(missingMix.sources.begin());
    require(!validateReferencePackage(missingMix).valid, "package without full mix must fail");

    auto duplicateMix = package;
    duplicateMix.sources.push_back({ "mix2", "Alt Full", "alt.wav", ReferenceRole::fullMix, 210.0, 48000.0, 2, true, false });
    require(!validateReferencePackage(duplicateMix).valid, "multiple authoritative mixes must fail");

    std::cout << "Reference model tests passed.\n";
    return EXIT_SUCCESS;
}
