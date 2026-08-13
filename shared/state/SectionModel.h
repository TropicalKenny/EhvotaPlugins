#pragma once

#include "ReferenceModel.h"

#include <optional>
#include <string>
#include <vector>

namespace ehvota::state {

struct SectionValidation {
    bool valid {};
    std::vector<ValidationIssue> issues;
};

[[nodiscard]] SectionValidation validateSections(
    const std::vector<SectionMarker>& sections,
    double mediaDurationSeconds);

[[nodiscard]] std::optional<SectionMarker> findSection(
    const std::vector<SectionMarker>& sections,
    const std::string& id);

[[nodiscard]] bool isValidSectionPair(
    const SectionPair& pair,
    const std::vector<SectionMarker>& mixSections,
    const std::vector<SectionMarker>& referenceSections) noexcept;

[[nodiscard]] double sectionDuration(const SectionMarker& section) noexcept;

} // namespace ehvota::state
