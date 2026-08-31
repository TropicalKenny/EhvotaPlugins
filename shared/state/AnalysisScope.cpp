#include "AnalysisScope.h"

#include <algorithm>

namespace ehvota::state {
namespace {

constexpr std::array<ReferenceRole, 4> diagnosticRoles {
    ReferenceRole::drums,
    ReferenceRole::bass,
    ReferenceRole::vocals,
    ReferenceRole::music
};

bool hasConnectedLiveSource(
    const MappingEntry& mapping,
    const std::vector<LiveSource>& liveSources)
{
    if (!mapping.enabled || mapping.liveSourceIds.empty())
        return false;

    for (const auto& sourceId : mapping.liveSourceIds) {
        const auto source = std::find_if(liveSources.begin(), liveSources.end(), [&](const auto& live) {
            return live.id == sourceId;
        });
        if (source == liveSources.end() || !source->connected)
            return false;
    }
    return true;
}

bool mappingHasMatchingReference(
    const MappingEntry& mapping,
    const ReferencePackage& reference,
    const ReferenceRole role)
{
    if (!mapping.enabled || mapping.role != role)
        return false;
    const auto source = std::find_if(reference.sources.begin(), reference.sources.end(), [&](const auto& candidate) {
        return candidate.id == mapping.referenceSourceId;
    });
    return source != reference.sources.end() && source->role == role;
}

} // namespace

bool AnalysisScope::includes(const ReferenceRole role) const noexcept
{
    switch (role) {
    case ReferenceRole::fullMix: return true;
    case ReferenceRole::drums: return drums;
    case ReferenceRole::bass: return bass;
    case ReferenceRole::vocals: return vocals;
    case ReferenceRole::music: return music;
    case ReferenceRole::other: return false;
    }
    return false;
}

void AnalysisScope::setIncluded(const ReferenceRole role, const bool included) noexcept
{
    switch (role) {
    case ReferenceRole::drums: drums = included; break;
    case ReferenceRole::bass: bass = included; break;
    case ReferenceRole::vocals: vocals = included; break;
    case ReferenceRole::music: music = included; break;
    case ReferenceRole::fullMix:
    case ReferenceRole::other:
        break;
    }
}

std::vector<ReferenceRole> AnalysisScope::enabledDiagnosticRoles() const
{
    std::vector<ReferenceRole> enabled;
    enabled.reserve(diagnosticRoles.size());
    for (const auto role : diagnosticRoles)
        if (includes(role))
            enabled.push_back(role);
    return enabled;
}

AnalysisScope AnalysisScope::only(const ReferenceRole role) noexcept
{
    auto scope = none();
    scope.setIncluded(role, true);
    return scope;
}

std::array<ScopeAvailability, 4> evaluateScopeAvailability(
    const AnalysisScope& scope,
    const ReferencePackage& reference,
    const std::vector<MappingEntry>& mappings,
    const std::vector<LiveSource>& liveSources)
{
    std::array<ScopeAvailability, 4> result {};
    for (std::size_t i = 0; i < diagnosticRoles.size(); ++i) {
        const auto role = diagnosticRoles[i];
        auto& availability = result[i];
        availability.role = role;
        availability.requested = scope.includes(role);

        const auto mapping = std::find_if(mappings.begin(), mappings.end(), [&](const auto& entry) {
            return entry.role == role && entry.enabled;
        });
        availability.hasReferenceStem = mapping != mappings.end()
            && mappingHasMatchingReference(*mapping, reference, role);
        availability.hasLiveMapping = mapping != mappings.end()
            && hasConnectedLiveSource(*mapping, liveSources);
        availability.usable = availability.requested
            && availability.hasReferenceStem
            && availability.hasLiveMapping;
    }
    return result;
}

} // namespace ehvota::state
