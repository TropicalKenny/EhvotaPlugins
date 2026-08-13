#include "MappingValidation.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace ehvota::state {

MappingValidation validateMappings(
    const ReferencePackage& reference,
    const std::vector<LiveSource>& liveSources,
    const std::vector<MappingEntry>& mappings)
{
    MappingValidation result { true, {} };
    std::unordered_set<int> mappedRoles;
    std::unordered_map<std::string, int> liveUseCount;

    for (const auto& mapping : mappings) {
        if (!mapping.enabled)
            continue;

        const auto roleKey = static_cast<int>(mapping.role);
        if (!mappedRoles.insert(roleKey).second) {
            result.valid = false;
            result.issues.push_back({ true, "Each semantic role may have only one enabled mapping entry." });
        }

        if (mapping.role == ReferenceRole::fullMix || mapping.role == ReferenceRole::other) {
            result.valid = false;
            result.issues.push_back({ true, "Mapping Matrix entries are for explicit diagnostic component roles only." });
            continue;
        }

        const auto referenceSource = std::find_if(reference.sources.begin(), reference.sources.end(), [&](const auto& source) {
            return source.id == mapping.referenceSourceId;
        });
        if (referenceSource == reference.sources.end()) {
            result.valid = false;
            result.issues.push_back({ true, "A mapping points to a reference source that no longer exists." });
        }
        else if (referenceSource->role != mapping.role) {
            result.valid = false;
            result.issues.push_back({ true, "A mapping's semantic role does not match its reference source role." });
        }

        if (mapping.liveSourceIds.empty()) {
            result.valid = false;
            result.issues.push_back({ true, "An enabled mapping requires at least one live Ehvota/Ableton source." });
            continue;
        }

        std::unordered_set<std::string> uniqueWithinMapping;
        for (const auto& liveId : mapping.liveSourceIds) {
            if (!uniqueWithinMapping.insert(liveId).second) {
                result.valid = false;
                result.issues.push_back({ true, "A live source is duplicated inside one mapping entry." });
                continue;
            }

            const auto live = std::find_if(liveSources.begin(), liveSources.end(), [&](const auto& source) {
                return source.id == liveId;
            });
            if (live == liveSources.end()) {
                result.valid = false;
                result.issues.push_back({ true, "A mapping points to a live source that no longer exists." });
                continue;
            }
            if (!live->connected)
                result.issues.push_back({ false, "A mapped live source is currently disconnected; the saved mapping is preserved for recovery." });
            ++liveUseCount[liveId];
        }
    }

    for (const auto& [liveId, count] : liveUseCount) {
        if (count > 1) {
            result.issues.push_back({ false,
                "Live source '" + liveId + "' participates in more than one semantic mapping; verify this is intentional." });
        }
    }

    return result;
}

} // namespace ehvota::state
