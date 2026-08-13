#include "shared/state/SectionModel.h"

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
    using ehvota::state::SectionMarker;
    using ehvota::state::SectionPair;

    const std::vector<SectionMarker> mix {
        { "verse-1", "Verse 1", 20.0, 52.0 },
        { "chorus-1", "Chorus 1", 52.0, 88.0 },
        { "verse-2", "Verse 2", 88.0, 120.0 }
    };
    const std::vector<SectionMarker> reference {
        { "ref-verse-1", "Verse 1", 18.0, 50.0 },
        { "ref-chorus-1", "Chorus 1", 50.0, 86.0 }
    };

    require(ehvota::state::validateSections(mix, 180.0).valid,
        "ordered non-overlapping sections within media should validate");
    require(ehvota::state::sectionDuration(mix[1]) == 36.0,
        "section duration should be end minus start");
    require(ehvota::state::findSection(mix, "chorus-1").has_value(),
        "section lookup should resolve stable ids");
    require(ehvota::state::isValidSectionPair(
        SectionPair { "chorus-1", "ref-chorus-1" }, mix, reference),
        "mix and reference sections may have different ids and timestamps when explicitly paired");

    const std::vector<SectionMarker> overlapping {
        { "a", "A", 0.0, 10.0 },
        { "b", "B", 9.0, 20.0 }
    };
    require(!ehvota::state::validateSections(overlapping, 30.0).valid,
        "overlapping Version 1 section ranges must be rejected rather than ambiguously analyzed");

    std::cout << "Section model tests passed\n";
    return 0;
}
