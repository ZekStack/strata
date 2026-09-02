#pragma once

#include "Capability.h"
#include "Placement.h"

#include <cstddef>
#include <optional>

namespace Strata {

struct MemoryStats {
    std::optional<std::size_t> totalBytes{};
    std::optional<std::size_t> freeBytes{};
    std::optional<std::size_t> minimumFreeBytes{};
    std::optional<std::size_t> largestFreeBlockBytes{};
};

[[nodiscard]] Region regionOf(const void *ptr) noexcept;
[[nodiscard]] bool supports(Placement placement) noexcept;
[[nodiscard]] bool supports(Region region) noexcept;
[[nodiscard]] bool supports(Capability capabilities) noexcept;
[[nodiscard]] MemoryStats memoryStats(Region region) noexcept;

} // namespace Strata
