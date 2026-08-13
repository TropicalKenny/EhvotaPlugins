#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ehvota::state {

enum class ReferenceRole {
    fullMix,
    drums,
    bass,
    vocals,
    music,
    other
};

struct ReferenceSource {
    std::string id;
    std::string displayName;
    std::string relativePath;
    ReferenceRole role { ReferenceRole::other };
    double durationSeconds {};
    double sampleRate {};
    int channels {};
    bool alignedToFullMix {};
    bool diagnosticOnly { true };
};

struct ReferencePackage {
    std::string id;
    std::string name;
    std::vector<ReferenceSource> sources;
};

struct LiveSource {
    std::string id;
    std::string displayName;
    std::string endpointKind;
    bool connected {};
};

struct MappingEntry {
    ReferenceRole role { ReferenceRole::other };
    std::string referenceSourceId;
    std::vector<std::string> liveSourceIds;
    bool enabled { true };
};

struct SectionMarker {
    std::string id;
    std::string label;
    double startSeconds {};
    double endSeconds {};
};

struct SectionPair {
    std::string mixSectionId;
    std::string referenceSectionId;
};

struct ValidationIssue {
    bool blocking {};
    std::string message;
};

struct ReferenceValidation {
    bool valid {};
    std::vector<ValidationIssue> issues;
};

ReferenceValidation validateReferencePackage(const ReferencePackage& package);
std::optional<ReferenceSource> authoritativeFullMix(const ReferencePackage& package);
bool isDiagnosticRole(ReferenceRole role) noexcept;
std::string_view referenceRoleName(ReferenceRole role) noexcept;

} // namespace ehvota::state
