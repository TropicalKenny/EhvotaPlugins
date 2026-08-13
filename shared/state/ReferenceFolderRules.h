#pragma once

#include "ReferenceModel.h"

#include <optional>
#include <string_view>

namespace ehvota::state {

[[nodiscard]] std::optional<ReferenceRole> inferReferenceRoleFromFilename(std::string_view filename);
[[nodiscard]] bool isSupportedReferenceAudioExtension(std::string_view filename) noexcept;

} // namespace ehvota::state
