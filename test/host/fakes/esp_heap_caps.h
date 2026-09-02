#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>

inline constexpr std::uint32_t MALLOC_CAP_8BIT = 1U << 0;
inline constexpr std::uint32_t MALLOC_CAP_INTERNAL = 1U << 1;
inline constexpr std::uint32_t MALLOC_CAP_SPIRAM = 1U << 2;

inline bool fake_heap_caps_fail_external = false;
inline std::uint32_t fake_heap_caps_last_caps = 0;
inline std::size_t fake_heap_caps_external_attempts = 0;
inline std::size_t fake_heap_caps_internal_attempts = 0;
inline std::size_t fake_heap_caps_aligned_attempts = 0;

inline void fake_heap_caps_reset() {
    fake_heap_caps_fail_external = false;
    fake_heap_caps_last_caps = 0;
    fake_heap_caps_external_attempts = 0;
    fake_heap_caps_internal_attempts = 0;
    fake_heap_caps_aligned_attempts = 0;
}

inline bool fake_heap_caps_should_fail(std::uint32_t caps) {
    fake_heap_caps_last_caps = caps;
    if ((caps & MALLOC_CAP_SPIRAM) != 0) {
        ++fake_heap_caps_external_attempts;
        return fake_heap_caps_fail_external;
    }
    if ((caps & MALLOC_CAP_INTERNAL) != 0) {
        ++fake_heap_caps_internal_attempts;
    }
    return false;
}

inline void *heap_caps_malloc(std::size_t size, std::uint32_t caps) {
    return fake_heap_caps_should_fail(caps) ? nullptr : std::malloc(size);
}

inline void *heap_caps_calloc(std::size_t count, std::size_t size, std::uint32_t caps) {
    return fake_heap_caps_should_fail(caps) ? nullptr : std::calloc(count, size);
}

inline void *heap_caps_realloc(void *ptr, std::size_t size, std::uint32_t caps) {
    return fake_heap_caps_should_fail(caps) ? nullptr : std::realloc(ptr, size);
}

inline void *heap_caps_aligned_alloc(std::size_t alignment, std::size_t size, std::uint32_t caps) {
    ++fake_heap_caps_aligned_attempts;
    if (fake_heap_caps_should_fail(caps)) {
        return nullptr;
    }

    const auto padding = alignment - 1;
    if (size > std::numeric_limits<std::size_t>::max() - padding) {
        return nullptr;
    }
    const auto roundedSize = (size + padding) & ~padding;
    return std::aligned_alloc(alignment, roundedSize);
}

inline void heap_caps_free(void *ptr) {
    std::free(ptr);
}
