#pragma once

#include "ReferenceModel.h"

#include <optional>
#include <string>
#include <vector>

namespace ehvota::state {

// One named musical region, with independent time ranges for the live mix and the reference.
// Ehvota never assumes that both songs place a chorus at the same timestamp or have identical length.
struct PairedSection {
    std::string id;
    std::string label;
    SectionMarker mix;
    SectionMarker reference;
    bool enabled { true };
};

struct PairedSectionValidation {
    bool valid {};
    std::vector<ValidationIssue> issues;
};

[[nodiscard]] PairedSectionValidation validatePairedSections(
    const std::vector<PairedSection>& sections,
    std::optional<double> mixDurationSeconds = std::nullopt,
    std::optional<double> referenceDurationSeconds = std::nullopt);

[[nodiscard]] std::optional<PairedSection> findPairedSection(
    const std::vector<PairedSection>& sections,
    const std::string& id);

[[nodiscard]] std::vector<PairedSection> enabledPairedSections(
    const std::vector<PairedSection>& sections);

[[nodiscard]] double pairedMixDuration(const PairedSection& section) noexcept;
[[nodiscard]] double pairedReferenceDuration(const PairedSection& section) noexcept;

} // namespace ehvota::state
