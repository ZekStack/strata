#include "Platform.h"

#if defined(ESP32)

#include <esp_heap_caps.h>
#include <esp_memory_utils.h>

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

[[nodiscard]] bool capsAvailable(std::uint32_t caps) noexcept {
    return heap_caps_get_total_size(caps) != 0;
}

[[nodiscard]] MemoryStats statsForCaps(std::uint32_t caps) noexcept {
    if (!capsAvailable(caps)) {
        return {};
    }

    return MemoryStats{
        .totalBytes = heap_caps_get_total_size(caps),
        .freeBytes = heap_caps_get_free_size(caps),
        .minimumFreeBytes = heap_caps_get_minimum_free_size(caps),
        .largestFreeBlockBytes = heap_caps_get_largest_free_block(caps),
    };
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

Region regionOf(const void *ptr) noexcept {
    if (ptr == nullptr) {
        return Region::Unknown;
    }
    if (esp_ptr_external_ram(ptr)) {
        return Region::External;
    }
    if (esp_ptr_internal(ptr)) {
        return Region::Internal;
    }
    return Region::Unknown;
}

bool supports(Placement placement) noexcept {
    const bool hasInternal = capsAvailable(InternalCaps);
    const bool hasExternal = capsAvailable(ExternalCaps);

    switch (placement) {
        case Placement::Default:
            return capsAvailable(DefaultCaps);
        case Placement::Internal:
            return hasInternal;
        case Placement::PreferExternal:
            return hasExternal || hasInternal;
        case Placement::RequireExternal:
            return hasExternal;
    }

    return false;
}

bool supports(Region region) noexcept {
    switch (region) {
        case Region::Unknown:
            return false;
        case Region::Internal:
            return capsAvailable(InternalCaps);
        case Region::External:
            return capsAvailable(ExternalCaps);
    }

    return false;
}

MemoryStats memoryStats(Region region) noexcept {
    switch (region) {
        case Region::Unknown:
            return {};
        case Region::Internal:
            return statsForCaps(InternalCaps);
        case Region::External:
            return statsForCaps(ExternalCaps);
    }

    return {};
}

} // namespace Strata::Internal

#endif
