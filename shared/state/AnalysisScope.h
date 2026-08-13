#pragma once

#include "ReferenceModel.h"

#include <array>
#include <vector>

namespace ehvota::state {

struct AnalysisScope {
    bool drums { true };
    bool bass { true };
    bool vocals { true };
    bool music { true };

    [[nodiscard]] bool includes(ReferenceRole role) const noexcept;
    void setIncluded(ReferenceRole role, bool included) noexcept;
    [[nodiscard]] std::vector<ReferenceRole> enabledDiagnosticRoles() const;

    [[nodiscard]] static AnalysisScope all() noexcept { return {}; }
    [[nodiscard]] static AnalysisScope none() noexcept { return { false, false, false, false }; }
    [[nodiscard]] static AnalysisScope only(ReferenceRole role) noexcept;
};

struct ScopeAvailability {
    ReferenceRole role { ReferenceRole::other };
    bool requested {};
    bool hasReferenceStem {};
    bool hasLiveMapping {};
    bool usable {};
};

[[nodiscard]] std::array<ScopeAvailability, 4> evaluateScopeAvailability(
    const AnalysisScope& scope,
    const ReferencePackage& reference,
    const std::vector<MappingEntry>& mappings,
    const std::vector<LiveSource>& liveSources);

} // namespace ehvota::state
