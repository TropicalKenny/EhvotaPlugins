#include "ReferenceFolderRules.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace ehvota::state {
namespace {

std::string normaliseStem(std::string_view filename)
{
    std::string value(filename);
    if (const auto dot = value.find_last_of('.'); dot != std::string::npos)
        value.resize(dot);

    std::string normalised;
    normalised.reserve(value.size());
    for (unsigned char ch : value) {
        if (std::isalnum(ch) != 0)
            normalised.push_back(static_cast<char>(std::tolower(ch)));
    }
    return normalised;
}

std::string lowerExtension(std::string_view filename)
{
    const auto dot = filename.find_last_of('.');
    if (dot == std::string_view::npos)
        return {};
    std::string ext(filename.substr(dot));
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return ext;
}

} // namespace

std::optional<ReferenceRole> inferReferenceRoleFromFilename(std::string_view filename)
{
    const auto stem = normaliseStem(filename);

    if (stem == "fullmix" || stem == "mix" || stem == "master" || stem == "reference" || stem == "ref")
        return ReferenceRole::fullMix;
    if (stem == "drums" || stem == "drum")
        return ReferenceRole::drums;
    if (stem == "bass")
        return ReferenceRole::bass;
    if (stem == "vocals" || stem == "vocal" || stem == "vox")
        return ReferenceRole::vocals;
    if (stem == "music" || stem == "instrumental" || stem == "instrumentals")
        return ReferenceRole::music;

    return std::nullopt;
}

bool isSupportedReferenceAudioExtension(std::string_view filename) noexcept
{
    const auto ext = lowerExtension(filename);
    return ext == ".wav" || ext == ".aif" || ext == ".aiff" || ext == ".flac";
}

} // namespace ehvota::state
