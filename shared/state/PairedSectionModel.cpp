#include "PairedSectionModel.h"

#include "SectionModel.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <unordered_set>

namespace ehvota::state {
namespace {

bool validOptionalDuration(const std::optional<double> value) noexcept
{
    return !value.has_value() || (std::isfinite(*value) && *value > 0.0);
}

} // namespace

PairedSectionValidation validatePairedSections(
    const std::vector<PairedSection>& sections,
    const std::optional<double> mixDurationSeconds,
    const std::optional<double> referenceDurationSeconds)
{
    PairedSectionValidation result { true, {} };
    if (!validOptionalDuration(mixDurationSeconds) || !validOptionalDuration(referenceDurationSeconds)) {
        result.valid = false;
        result.issues.push_back({ true, "Known media durations must be finite and positive." });
        return result;
    }

    std::unordered_set<std::string> ids;
    std::vector<SectionMarker> mix;
    std::vector<SectionMarker> reference;
    mix.reserve(sections.size());
    reference.reserve(sections.size());

    for (const auto& section : sections) {
        if (section.id.empty() || section.label.empty()) {
            result.valid = false;
            result.issues.push_back({ true, "Every paired section requires a stable id and visible label." });
        }
        if (!ids.insert(section.id).second) {
            result.valid = false;
            result.issues.push_back({ true, "Paired section ids must be unique." });
        }

        auto mixMarker = section.mix;
        auto referenceMarker = section.reference;
        mixMarker.id = section.id + ":mix";
        referenceMarker.id = section.id + ":reference";
        if (mixMarker.label.empty()) mixMarker.label = section.label;
        if (referenceMarker.label.empty()) referenceMarker.label = section.label;
        mix.push_back(std::move(mixMarker));
        reference.push_back(std::move(referenceMarker));

        const auto mixDuration = pairedMixDuration(section);
        const auto refDuration = pairedReferenceDuration(section);
        if (mixDuration > 0.0 && refDuration > 0.0) {
            const auto larger = std::max(mixDuration, refDuration);
            const auto smaller = std::min(mixDuration, refDuration);
            if (smaller / larger < 0.50) {
                result.issues.push_back({ false,
                    "A paired section differs by more than 2x in duration; verify that the musical regions really correspond." });
            }
        }
    }

    const auto inferredMixDuration = [&] {
        if (mixDurationSeconds) return *mixDurationSeconds;
        double end = 0.0;
        for (const auto& marker : mix) end = std::max(end, marker.endSeconds);
        return end;
    }();
    const auto inferredReferenceDuration = [&] {
        if (referenceDurationSeconds) return *referenceDurationSeconds;
        double end = 0.0;
        for (const auto& marker : reference) end = std::max(end, marker.endSeconds);
        return end;
    }();

    if (!mix.empty()) {
        const auto validation = validateSections(mix, inferredMixDuration);
        for (const auto& issue : validation.issues) {
            result.issues.push_back({ issue.blocking, "Mix: " + issue.message });
            if (issue.blocking) result.valid = false;
        }
    }
    if (!reference.empty()) {
        const auto validation = validateSections(reference, inferredReferenceDuration);
        for (const auto& issue : validation.issues) {
            result.issues.push_back({ issue.blocking, "Reference: " + issue.message });
            if (issue.blocking) result.valid = false;
        }
    }

    return result;
}

std::optional<PairedSection> findPairedSection(
    const std::vector<PairedSection>& sections,
    const std::string& id)
{
    const auto it = std::find_if(sections.begin(), sections.end(), [&](const auto& section) {
        return section.id == id;
    });
    if (it == sections.end()) return std::nullopt;
    return *it;
}

std::vector<PairedSection> enabledPairedSections(const std::vector<PairedSection>& sections)
{
    std::vector<PairedSection> output;
    output.reserve(sections.size());
    std::copy_if(sections.begin(), sections.end(), std::back_inserter(output),
        [](const auto& section) { return section.enabled; });
    return output;
}

double pairedMixDuration(const PairedSection& section) noexcept
{
    return section.mix.endSeconds > section.mix.startSeconds
        ? section.mix.endSeconds - section.mix.startSeconds : 0.0;
}

double pairedReferenceDuration(const PairedSection& section) noexcept
{
    return section.reference.endSeconds > section.reference.startSeconds
        ? section.reference.endSeconds - section.reference.startSeconds : 0.0;
}

} // namespace ehvota::state
