#include "ReferenceFolderLoader.h"

#include "shared/state/ReferenceFolderRules.h"

#include <cstdint>
#include <sstream>

namespace ehvota::refhub {
namespace {

std::string stableId(std::string_view text)
{
    std::uint64_t hash = 1469598103934665603ull;
    for (unsigned char ch : text) {
        hash ^= ch;
        hash *= 1099511628211ull;
    }
    std::ostringstream out;
    out << std::hex << hash;
    return out.str();
}

} // namespace

ReferenceFolderLoadResult loadReferenceFolder(
    const juce::File& directory,
    juce::AudioFormatManager& formatManager)
{
    ReferenceFolderLoadResult result;
    result.package.name = directory.getFileName().toStdString();
    result.package.id = stableId(directory.getFullPathName().toStdString());

    std::vector<ehvota::state::ValidationIssue> discoveryIssues;
    if (!directory.isDirectory()) {
        result.validation.valid = false;
        result.validation.issues.push_back({ true, "Selected reference path is not a directory." });
        return result;
    }

    const auto children = directory.findChildFiles(juce::File::findFiles, false);
    for (const auto& file : children) {
        const auto filename = file.getFileName().toStdString();
        if (!ehvota::state::isSupportedReferenceAudioExtension(filename))
            continue;

        const auto role = ehvota::state::inferReferenceRoleFromFilename(filename);
        if (!role) {
            discoveryIssues.push_back({ false,
                "Audio file '" + filename + "' has no deterministic semantic role and was left unassigned." });
            continue;
        }

        auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
        if (!reader) {
            discoveryIssues.push_back({ true, "Could not read reference audio file '" + filename + "'." });
            continue;
        }

        ehvota::state::ReferenceSource source;
        source.id = stableId(file.getFullPathName().toStdString());
        source.displayName = file.getFileNameWithoutExtension().toStdString();
        source.relativePath = filename;
        source.role = *role;
        source.sampleRate = reader->sampleRate;
        source.channels = static_cast<int>(reader->numChannels);
        source.durationSeconds = reader->sampleRate > 0.0
            ? static_cast<double>(reader->lengthInSamples) / reader->sampleRate
            : 0.0;
        source.alignedToFullMix = source.role == ehvota::state::ReferenceRole::fullMix;
        source.diagnosticOnly = source.role != ehvota::state::ReferenceRole::fullMix;

        if (source.role == ehvota::state::ReferenceRole::fullMix)
            result.authoritativeFullMixFile = file;

        result.package.sources.push_back(std::move(source));
        result.files.push_back(file);
    }

    result.validation = ehvota::state::validateReferencePackage(result.package);
    for (auto& issue : discoveryIssues) {
        if (issue.blocking)
            result.validation.valid = false;
        result.validation.issues.push_back(std::move(issue));
    }

    return result;
}

} // namespace ehvota::refhub
