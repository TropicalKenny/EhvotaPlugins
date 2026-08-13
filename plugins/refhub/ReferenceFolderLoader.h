#pragma once

#include "shared/state/ReferenceModel.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <vector>

namespace ehvota::refhub {

struct ReferenceFolderLoadResult {
    ehvota::state::ReferencePackage package;
    ehvota::state::ReferenceValidation validation;
    std::vector<juce::File> files;
    juce::File authoritativeFullMixFile;
};

[[nodiscard]] ReferenceFolderLoadResult loadReferenceFolder(
    const juce::File& directory,
    juce::AudioFormatManager& formatManager);

} // namespace ehvota::refhub
