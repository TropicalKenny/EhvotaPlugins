#include "shared/state/PairedSectionModel.h"

#include <cassert>
#include <cmath>
#include <vector>

int main()
{
    using namespace ehvota::state;

    std::vector<PairedSection> sections {
        { "intro", "Intro", { {}, {}, 0.0, 12.0 }, { {}, {}, 4.0, 16.0 }, true },
        { "verse1", "Verse 1", { {}, {}, 12.0, 32.0 }, { {}, {}, 16.0, 38.0 }, true },
        { "chorus1", "Chorus 1", { {}, {}, 32.0, 48.0 }, { {}, {}, 38.0, 54.0 }, true },
        { "verse2", "Verse 2", { {}, {}, 48.0, 68.0 }, { {}, {}, 54.0, 76.0 }, false }
    };

    const auto validation = validatePairedSections(sections, 120.0, 130.0);
    assert(validation.valid);
    assert(findPairedSection(sections, "chorus1").has_value());
    assert(!findPairedSection(sections, "missing").has_value());
    assert(enabledPairedSections(sections).size() == 3);
    assert(std::abs(pairedMixDuration(sections[2]) - 16.0) < 1.0e-9);
    assert(std::abs(pairedReferenceDuration(sections[1]) - 22.0) < 1.0e-9);

    auto overlap = sections;
    overlap[1].mix.startSeconds = 10.0;
    assert(!validatePairedSections(overlap, 120.0, 130.0).valid);

    auto duplicate = sections;
    duplicate[1].id = "intro";
    assert(!validatePairedSections(duplicate, 120.0, 130.0).valid);

    auto beyond = sections;
    beyond[2].reference.endSeconds = 200.0;
    assert(!validatePairedSections(beyond, 120.0, 130.0).valid);

    auto veryDifferent = sections;
    veryDifferent[0].reference.endSeconds = 34.0;
    const auto warning = validatePairedSections(veryDifferent, 120.0, 130.0);
    bool sawNonBlocking = false;
    for (const auto& issue : warning.issues)
        sawNonBlocking = sawNonBlocking || !issue.blocking;
    assert(sawNonBlocking);

    return 0;
}
