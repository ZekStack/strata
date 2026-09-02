#pragma once

#include "../Capability.h"
#include "../Diagnostics.h"
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
    Placement placement,
    Capability capabilities) noexcept;
[[nodiscard]] void *calloc(
    std::size_t count,
    std::size_t sizeBytes,
    Placement placement) noexcept;
[[nodiscard]] void *reallocate(
    void *ptr,
    std::size_t newSizeBytes,
    Placement placement) noexcept;
void free(void *ptr) noexcept;

[[nodiscard]] Region regionOf(const void *ptr) noexcept;
[[nodiscard]] bool supports(Placement placement) noexcept;
[[nodiscard]] bool supports(Region region) noexcept;
[[nodiscard]] bool supports(Capability capabilities) noexcept;
[[nodiscard]] MemoryStats memoryStats(Region region) noexcept;

} // namespace Strata::Internal
