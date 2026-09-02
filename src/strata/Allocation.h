#pragma once

#include "Placement.h"

#include <cstddef>

namespace Strata {

inline constexpr std::size_t DefaultAlignment = alignof(std::max_align_t);

struct AllocationRequest {
    std::size_t sizeBytes{0};
    Placement placement{Placement::Default};
    std::size_t alignment{DefaultAlignment};
};

[[nodiscard]] void *allocate(std::size_t sizeBytes, Placement placement = Placement::Default) noexcept;
[[nodiscard]] void *allocate(const AllocationRequest &request) noexcept;
[[nodiscard]] void *calloc(
    std::size_t count,
    std::size_t sizeBytes,
    Placement placement = Placement::Default) noexcept;
[[nodiscard]] void *reallocate(
    void *ptr,
    std::size_t newSizeBytes,
    Placement placement = Placement::Default) noexcept;
void free(void *ptr) noexcept;

} // namespace Strata
