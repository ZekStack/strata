#include "Diagnostics.h"

#include "internal/Platform.h"

namespace Strata {

Region regionOf(const void *ptr) noexcept {
    if (ptr == nullptr) {
        return Region::Unknown;
    }

    return Internal::regionOf(ptr);
}

bool supports(Placement placement) noexcept {
    return Internal::supports(placement);
}

bool supports(Region region) noexcept {
    return Internal::supports(region);
}

MemoryStats memoryStats(Region region) noexcept {
    return Internal::memoryStats(region);
}

} // namespace Strata
