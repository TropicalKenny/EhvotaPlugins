#pragma once

#include "shared/state/ReferenceModel.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <optional>
#include <vector>

namespace ehvota::refhub {

struct ReferenceFolderLoadResult {
    ehvota::state::ReferencePackage package;
    ehvota::state::ReferenceValidation validation;
    std::vector<juce::File> files;
    juce::File authoritativeFullMixFile;
};

[[nodiscard]] ReferenceFolderLoadResult loadReferenceFolder(const juce::File& directory);
[[nodiscard]] std::optional<juce::File> referenceFileForRole(
    const ReferenceFolderLoadResult& result,
    ehvota::state::ReferenceRole role);
[[nodiscard]] std::optional<ehvota::state::ReferenceSource> referenceSourceForRole(
    const ReferenceFolderLoadResult& result,
    ehvota::state::ReferenceRole role);

} // namespace ehvota::refhub
