#pragma once

#include "ReferenceModel.h"

#include <vector>

namespace ehvota::state {

struct MappingValidation {
    bool valid {};
    std::vector<ValidationIssue> issues;
};

[[nodiscard]] MappingValidation validateMappings(
    const ReferencePackage& reference,
    const std::vector<LiveSource>& liveSources,
    const std::vector<MappingEntry>& mappings);

} // namespace ehvota::state
