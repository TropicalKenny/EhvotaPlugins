#include "SectionModel.h"

#include <algorithm>
#include <unordered_set>

namespace ehvota::state {

SectionValidation validateSections(
    const std::vector<SectionMarker>& sections,
    const double mediaDurationSeconds)
{
    SectionValidation result { true, {} };
    if (!(mediaDurationSeconds > 0.0)) {
        result.valid = false;
        result.issues.push_back({ true, "Media duration must be positive before sections can be validated." });
        return result;
    }

    std::unordered_set<std::string> ids;
    double previousEnd = 0.0;
    for (const auto& section : sections) {
        if (section.id.empty() || section.label.empty()) {
            result.valid = false;
            result.issues.push_back({ true, "Every section requires a stable id and visible label." });
        }
        if (!ids.insert(section.id).second) {
            result.valid = false;
            result.issues.push_back({ true, "Section ids must be unique." });
        }
        if (!(section.startSeconds >= 0.0) || !(section.endSeconds > section.startSeconds)) {
            result.valid = false;
            result.issues.push_back({ true, "Section ranges require start >= 0 and end > start." });
            continue;
        }
        if (section.endSeconds > mediaDurationSeconds + 0.001) {
            result.valid = false;
            result.issues.push_back({ true, "A section extends beyond the available media duration." });
        }
        if (section.startSeconds + 0.001 < previousEnd) {
            result.valid = false;
            result.issues.push_back({ true, "Sections must not overlap in the current Version 1 section model." });
        }
        previousEnd = std::max(previousEnd, section.endSeconds);
    }

    return result;
}

std::optional<SectionMarker> findSection(
    const std::vector<SectionMarker>& sections,
    const std::string& id)
{
    const auto it = std::find_if(sections.begin(), sections.end(), [&](const auto& section) {
        return section.id == id;
    });
    if (it == sections.end())
        return std::nullopt;
    return *it;
}

bool isValidSectionPair(
    const SectionPair& pair,
    const std::vector<SectionMarker>& mixSections,
    const std::vector<SectionMarker>& referenceSections) noexcept
{
    const auto mix = std::find_if(mixSections.begin(), mixSections.end(), [&](const auto& section) {
        return section.id == pair.mixSectionId;
    });
    const auto reference = std::find_if(referenceSections.begin(), referenceSections.end(), [&](const auto& section) {
        return section.id == pair.referenceSectionId;
    });
    return mix != mixSections.end() && reference != referenceSections.end();
}

double sectionDuration(const SectionMarker& section) noexcept
{
    return section.endSeconds > section.startSeconds
        ? section.endSeconds - section.startSeconds
        : 0.0;
}

} // namespace ehvota::state
