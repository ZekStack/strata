#include "Platform.h"

#if !defined(ESP32)

#include <cstdlib>
#include <limits>

namespace Strata::Internal {
namespace {

[[nodiscard]] void *allocateAligned(std::size_t sizeBytes, std::size_t alignment) noexcept {
    if (alignment <= alignof(std::max_align_t)) {
        return std::malloc(sizeBytes);
    }

    const auto padding = alignment - 1;
    if (sizeBytes > std::numeric_limits<std::size_t>::max() - padding) {
        return nullptr;
    }

    const auto roundedSize = (sizeBytes + padding) & ~padding;
    return std::aligned_alloc(alignment, roundedSize);
}

} // namespace

PlatformKind platformKind() noexcept {
    return PlatformKind::Generic;
}

const char *platformName() noexcept {
    return "generic";
}

void *allocate(std::size_t sizeBytes, std::size_t alignment, Placement placement) noexcept {
    if (placement == Placement::RequireExternal) {
        return nullptr;
    }

    return allocateAligned(sizeBytes, alignment);
}

void *calloc(std::size_t count, std::size_t sizeBytes, Placement placement) noexcept {
    if (placement == Placement::RequireExternal) {
        return nullptr;
    }

    return std::calloc(count, sizeBytes);
}

void *reallocate(void *ptr, std::size_t newSizeBytes, Placement placement) noexcept {
    if (placement == Placement::RequireExternal) {
        return nullptr;
    }

    return std::realloc(ptr, newSizeBytes);
}

void free(void *ptr) noexcept {
    std::free(ptr);
}

} // namespace Strata::Internal

#endif
