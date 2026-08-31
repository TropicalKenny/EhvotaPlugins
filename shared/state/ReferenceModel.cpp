#include "ReferenceModel.h"

#include <cmath>
#include <unordered_set>

namespace ehvota::state {

ReferenceValidation validateReferencePackage(const ReferencePackage& package)
{
    ReferenceValidation result { true, {} };
    if (package.id.empty() || package.name.empty()) {
        result.valid = false;
        result.issues.push_back({ true, "Reference package requires an id and display name." });
    }

    std::unordered_set<std::string> ids;
    int fullMixCount = 0;
    std::optional<double> fullDuration;
    std::optional<double> fullSampleRate;

    for (const auto& source : package.sources) {
        if (source.id.empty() || source.relativePath.empty()) {
            result.valid = false;
            result.issues.push_back({ true, "Every reference source requires an id and relative path." });
        }
        if (!ids.insert(source.id).second) {
            result.valid = false;
            result.issues.push_back({ true, "Reference source ids must be unique." });
        }
        if (source.channels <= 0 || source.sampleRate <= 0.0 || source.durationSeconds <= 0.0) {
            result.valid = false;
            result.issues.push_back({ true, "Reference source metadata is incomplete." });
        }
        if (source.role == ReferenceRole::fullMix) {
            ++fullMixCount;
            fullDuration = source.durationSeconds;
            fullSampleRate = source.sampleRate;
            if (source.diagnosticOnly)
                result.issues.push_back({ false, "Full mix is authoritative and should not be marked diagnostic-only." });
        }
    }

    if (fullMixCount != 1) {
        result.valid = false;
        result.issues.push_back({ true, "Exactly one authoritative full mix is required." });
    }

    if (fullDuration.has_value() && fullSampleRate.has_value()) {
        for (const auto& source : package.sources) {
            if (source.role == ReferenceRole::fullMix)
                continue;
            if (std::abs(source.durationSeconds - *fullDuration) > 0.050)
                result.issues.push_back({ false, "A diagnostic stem duration differs from the authoritative full mix by more than 50 ms." });
            if (std::abs(source.sampleRate - *fullSampleRate) > 0.5)
                result.issues.push_back({ false, "A diagnostic stem sample rate differs from the authoritative full mix." });
            if (!source.alignedToFullMix)
                result.issues.push_back({ false, "A diagnostic stem has not been confirmed sample-aligned to the full mix." });
        }
    }

    return result;
}

std::optional<ReferenceSource> authoritativeFullMix(const ReferencePackage& package)
{
    for (const auto& source : package.sources)
        if (source.role == ReferenceRole::fullMix)
            return source;
    return std::nullopt;
}

bool isDiagnosticRole(const ReferenceRole role) noexcept
{
    return role != ReferenceRole::fullMix;
}

std::string_view referenceRoleName(const ReferenceRole role) noexcept
{
    switch (role) {
    case ReferenceRole::fullMix: return "Full Mix";
    case ReferenceRole::drums: return "Drums";
    case ReferenceRole::bass: return "Bass";
    case ReferenceRole::vocals: return "Vocals";
    case ReferenceRole::music: return "Music";
    case ReferenceRole::other: return "Other";
    }
    return "Other";
}

} // namespace ehvota::state
