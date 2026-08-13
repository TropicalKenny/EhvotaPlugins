#include "shared/state/ReferenceFolderRules.h"

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
    using ehvota::state::ReferenceRole;
    using ehvota::state::inferReferenceRoleFromFilename;
    using ehvota::state::isSupportedReferenceAudioExtension;

    require(inferReferenceRoleFromFilename("full_mix.wav") == ReferenceRole::fullMix,
        "full_mix.wav should map to the authoritative full mix");
    require(inferReferenceRoleFromFilename("DRUMS.WAV") == ReferenceRole::drums,
        "role inference should be case-insensitive");
    require(inferReferenceRoleFromFilename("bass.flac") == ReferenceRole::bass,
        "bass.flac should map to bass");
    require(inferReferenceRoleFromFilename("vox.aiff") == ReferenceRole::vocals,
        "vox.aiff should map to vocals");
    require(inferReferenceRoleFromFilename("instrumental.wav") == ReferenceRole::music,
        "instrumental.wav should map to music");
    require(!inferReferenceRoleFromFilename("guitar.wav").has_value(),
        "uncertain filenames must not be silently assigned a semantic role");

    require(isSupportedReferenceAudioExtension("reference.wav"), "WAV should be supported");
    require(isSupportedReferenceAudioExtension("reference.FLAC"), "FLAC should be supported case-insensitively");
    require(!isSupportedReferenceAudioExtension("reference.mp3"),
        "MP3 is intentionally excluded from the deterministic reference package contract for now");

    std::cout << "Reference folder rules tests passed\n";
    return 0;
}
