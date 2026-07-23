#pragma once

namespace ehvota::preview {

enum class LayoutClass {
    compact,
    standard,
    wide
};

[[nodiscard]] LayoutClass classifyLayout(int logicalWidth);

} // namespace ehvota::preview
