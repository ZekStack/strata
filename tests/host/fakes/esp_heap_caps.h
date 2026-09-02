#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <unordered_map>

inline constexpr std::uint32_t MALLOC_CAP_8BIT = 1U << 0;
inline constexpr std::uint32_t MALLOC_CAP_INTERNAL = 1U << 1;
inline constexpr std::uint32_t MALLOC_CAP_SPIRAM = 1U << 2;
inline constexpr std::uint32_t MALLOC_CAP_DMA = 1U << 3;
inline constexpr std::uint32_t MALLOC_CAP_EXEC = 1U << 4;

inline bool fake_heap_caps_fail_external = false;
inline std::uint32_t fake_heap_caps_last_caps = 0;
inline std::size_t fake_heap_caps_external_attempts = 0;
inline std::size_t fake_heap_caps_internal_attempts = 0;
inline std::size_t fake_heap_caps_aligned_attempts = 0;
inline std::size_t fake_heap_caps_internal_total = 128 * 1024;
inline std::size_t fake_heap_caps_internal_free = 96 * 1024;
inline std::size_t fake_heap_caps_internal_minimum_free = 80 * 1024;
inline std::size_t fake_heap_caps_internal_largest = 64 * 1024;
inline std::size_t fake_heap_caps_external_total = 512 * 1024;
inline std::size_t fake_heap_caps_external_free = 448 * 1024;
inline std::size_t fake_heap_caps_external_minimum_free = 400 * 1024;
inline std::size_t fake_heap_caps_external_largest = 256 * 1024;
inline std::unordered_map<const void *, std::uint32_t> fake_heap_caps_allocations{};

inline void fake_heap_caps_reset() {
    fake_heap_caps_fail_external = false;
    fake_heap_caps_last_caps = 0;
    fake_heap_caps_external_attempts = 0;
    fake_heap_caps_internal_attempts = 0;
    fake_heap_caps_aligned_attempts = 0;
    fake_heap_caps_internal_total = 128 * 1024;
    fake_heap_caps_internal_free = 96 * 1024;
    fake_heap_caps_internal_minimum_free = 80 * 1024;
    fake_heap_caps_internal_largest = 64 * 1024;
    fake_heap_caps_external_total = 512 * 1024;
    fake_heap_caps_external_free = 448 * 1024;
    fake_heap_caps_external_minimum_free = 400 * 1024;
    fake_heap_caps_external_largest = 256 * 1024;
    fake_heap_caps_allocations.clear();
}

inline bool fake_heap_caps_is_external(std::uint32_t caps) {
    return (caps & MALLOC_CAP_SPIRAM) != 0;
}

inline bool fake_heap_caps_is_internal(std::uint32_t caps) {
    return (caps & MALLOC_CAP_INTERNAL) != 0 || !fake_heap_caps_is_external(caps);
}

inline bool fake_heap_caps_incompatible(std::uint32_t caps) {
    const bool external = fake_heap_caps_is_external(caps);
    const bool dma = (caps & MALLOC_CAP_DMA) != 0;
    const bool executable = (caps & MALLOC_CAP_EXEC) != 0;
    return (external && (dma || executable)) || (dma && executable);
}

inline bool fake_heap_caps_should_fail(std::uint32_t caps) {
    fake_heap_caps_last_caps = caps;
    if (fake_heap_caps_is_external(caps)) {
        ++fake_heap_caps_external_attempts;
        if (fake_heap_caps_fail_external || fake_heap_caps_external_total == 0) {
            return true;
        }
    }
    if ((caps & MALLOC_CAP_INTERNAL) != 0) {
        ++fake_heap_caps_internal_attempts;
    }
    return fake_heap_caps_incompatible(caps);
}

inline void fake_heap_caps_record(void *ptr, std::uint32_t caps) {
    if (ptr != nullptr) {
        fake_heap_caps_allocations[ptr] = caps;
    }
}

inline void *heap_caps_malloc(std::size_t size, std::uint32_t caps) {
    if (fake_heap_caps_should_fail(caps)) return nullptr;
    auto *ptr = std::malloc(size);
    fake_heap_caps_record(ptr, caps);
    return ptr;
}

inline void *heap_caps_calloc(std::size_t count, std::size_t size, std::uint32_t caps) {
    if (fake_heap_caps_should_fail(caps)) return nullptr;
    auto *ptr = std::calloc(count, size);
    fake_heap_caps_record(ptr, caps);
    return ptr;
}

inline void *heap_caps_realloc(void *ptr, std::size_t size, std::uint32_t caps) {
    if (fake_heap_caps_should_fail(caps)) return nullptr;
    const auto existing = fake_heap_caps_allocations.find(ptr);
    auto *resized = std::realloc(ptr, size);
    if (resized != nullptr) {
        if (existing != fake_heap_caps_allocations.end()) fake_heap_caps_allocations.erase(existing);
        fake_heap_caps_allocations[resized] = caps;
    }
    return resized;
}

inline void *heap_caps_aligned_alloc(std::size_t alignment, std::size_t size, std::uint32_t caps) {
    ++fake_heap_caps_aligned_attempts;
    if (fake_heap_caps_should_fail(caps)) return nullptr;
    const auto padding = alignment - 1;
    if (size > std::numeric_limits<std::size_t>::max() - padding) return nullptr;
    const auto roundedSize = (size + padding) & ~padding;
    auto *ptr = std::aligned_alloc(alignment, roundedSize);
    fake_heap_caps_record(ptr, caps);
    return ptr;
}

inline void heap_caps_free(void *ptr) {
    fake_heap_caps_allocations.erase(ptr);
    std::free(ptr);
}

inline std::size_t heap_caps_get_total_size(std::uint32_t caps) {
    if (fake_heap_caps_incompatible(caps)) return 0;
    if (fake_heap_caps_is_external(caps)) return fake_heap_caps_external_total;
    return fake_heap_caps_internal_total;
}

inline std::size_t heap_caps_get_free_size(std::uint32_t caps) {
    if (fake_heap_caps_incompatible(caps)) return 0;
    return fake_heap_caps_is_external(caps) ? fake_heap_caps_external_free : fake_heap_caps_internal_free;
}

inline std::size_t heap_caps_get_minimum_free_size(std::uint32_t caps) {
    if (fake_heap_caps_incompatible(caps)) return 0;
    return fake_heap_caps_is_external(caps) ? fake_heap_caps_external_minimum_free : fake_heap_caps_internal_minimum_free;
}

inline std::size_t heap_caps_get_largest_free_block(std::uint32_t caps) {
    if (fake_heap_caps_incompatible(caps)) return 0;
    return fake_heap_caps_is_external(caps) ? fake_heap_caps_external_largest : fake_heap_caps_internal_largest;
}
