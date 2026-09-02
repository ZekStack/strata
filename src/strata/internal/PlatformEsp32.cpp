#include "Platform.h"

#if defined(ESP32)

#include <esp_heap_caps.h>

namespace Strata::Internal {
namespace {

constexpr std::uint32_t DefaultCaps = MALLOC_CAP_8BIT;
constexpr std::uint32_t InternalCaps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
constexpr std::uint32_t ExternalCaps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;

[[nodiscard]] void *allocateWithCaps(
    std::size_t sizeBytes,
    std::size_t alignment,
    std::uint32_t caps) noexcept {
    if (alignment <= alignof(std::max_align_t)) {
        return heap_caps_malloc(sizeBytes, caps);
    }

    return heap_caps_aligned_alloc(alignment, sizeBytes, caps);
}

[[nodiscard]] void *callocWithCaps(
    std::size_t count,
    std::size_t sizeBytes,
    std::uint32_t caps) noexcept {
    return heap_caps_calloc(count, sizeBytes, caps);
}

[[nodiscard]] void *reallocateWithCaps(
    void *ptr,
    std::size_t newSizeBytes,
    std::uint32_t caps) noexcept {
    return heap_caps_realloc(ptr, newSizeBytes, caps);
}

} // namespace

PlatformKind platformKind() noexcept {
    return PlatformKind::Esp32;
}

const char *platformName() noexcept {
    return "esp32";
}

void *allocate(std::size_t sizeBytes, std::size_t alignment, Placement placement) noexcept {
    switch (placement) {
        case Placement::Default:
            return allocateWithCaps(sizeBytes, alignment, DefaultCaps);
        case Placement::Internal:
            return allocateWithCaps(sizeBytes, alignment, InternalCaps);
        case Placement::PreferExternal: {
            auto *ptr = allocateWithCaps(sizeBytes, alignment, ExternalCaps);
            return ptr != nullptr ? ptr : allocateWithCaps(sizeBytes, alignment, InternalCaps);
        }
        case Placement::RequireExternal:
            return allocateWithCaps(sizeBytes, alignment, ExternalCaps);
    }

    return nullptr;
}

void *calloc(std::size_t count, std::size_t sizeBytes, Placement placement) noexcept {
    switch (placement) {
        case Placement::Default:
            return callocWithCaps(count, sizeBytes, DefaultCaps);
        case Placement::Internal:
            return callocWithCaps(count, sizeBytes, InternalCaps);
        case Placement::PreferExternal: {
            auto *ptr = callocWithCaps(count, sizeBytes, ExternalCaps);
            return ptr != nullptr ? ptr : callocWithCaps(count, sizeBytes, InternalCaps);
        }
        case Placement::RequireExternal:
            return callocWithCaps(count, sizeBytes, ExternalCaps);
    }

    return nullptr;
}

void *reallocate(void *ptr, std::size_t newSizeBytes, Placement placement) noexcept {
    switch (placement) {
        case Placement::Default:
            return reallocateWithCaps(ptr, newSizeBytes, DefaultCaps);
        case Placement::Internal:
            return reallocateWithCaps(ptr, newSizeBytes, InternalCaps);
        case Placement::PreferExternal: {
            auto *resized = reallocateWithCaps(ptr, newSizeBytes, ExternalCaps);
            return resized != nullptr ? resized : reallocateWithCaps(ptr, newSizeBytes, InternalCaps);
        }
        case Placement::RequireExternal:
            return reallocateWithCaps(ptr, newSizeBytes, ExternalCaps);
    }

    return nullptr;
}

void free(void *ptr) noexcept {
    heap_caps_free(ptr);
}

} // namespace Strata::Internal

#endif
