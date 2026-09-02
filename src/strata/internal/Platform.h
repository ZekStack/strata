#pragma once

#include "../Placement.h"

#include <cstddef>
#include <cstdint>

namespace Strata::Internal {

enum class PlatformKind : std::uint8_t {
    Generic,
    Esp32,
};

[[nodiscard]] PlatformKind platformKind() noexcept;
[[nodiscard]] const char *platformName() noexcept;

[[nodiscard]] void *allocate(
    std::size_t sizeBytes,
    std::size_t alignment,
    Placement placement) noexcept;
[[nodiscard]] void *calloc(
    std::size_t count,
    std::size_t sizeBytes,
    Placement placement) noexcept;
[[nodiscard]] void *reallocate(
    void *ptr,
    std::size_t newSizeBytes,
    Placement placement) noexcept;
void free(void *ptr) noexcept;

} // namespace Strata::Internal
