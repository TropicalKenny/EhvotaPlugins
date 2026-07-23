#include "LayoutPolicy.h"

#include <stdexcept>

namespace ehvota::preview {

LayoutClass classifyLayout(const int logicalWidth)
{
    if (logicalWidth <= 0)
        throw std::invalid_argument("Logical width must be greater than zero.");
    if (logicalWidth < 760)
        return LayoutClass::compact;
    if (logicalWidth < 1'100)
        return LayoutClass::standard;
    return LayoutClass::wide;
}

} // namespace ehvota::preview
