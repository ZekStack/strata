#pragma once

#include "Capability.h"
#include "Placement.h"

#include <cstddef>
#include <optional>

#ifndef STRATA_ENABLE_ADVANCED_DIAGNOSTICS
#define STRATA_ENABLE_ADVANCED_DIAGNOSTICS 0
#endif

#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS != 0 && STRATA_ENABLE_ADVANCED_DIAGNOSTICS != 1
#error "STRATA_ENABLE_ADVANCED_DIAGNOSTICS must be 0 or 1"
#endif

namespace Strata {

struct MemoryStats {
	std::optional<std::size_t> totalBytes{};
	std::optional<std::size_t> freeBytes{};
	std::optional<std::size_t> minimumFreeBytes{};
	std::optional<std::size_t> largestFreeBlockBytes{};
	std::optional<std::size_t> usedBytes{};
	std::optional<std::size_t> peakUsedBytes{};
};

struct AllocationDiagnostics {
	std::size_t attempts{0};
	std::size_t successes{0};
	std::size_t failures{0};
	std::size_t invalidRequests{0};
	std::size_t preferredExternalFallbacks{0};
	std::size_t requestedBytes{0};
	std::size_t successfulBytes{0};
	std::size_t failedBytes{0};
};

inline constexpr bool AdvancedDiagnosticsEnabled = STRATA_ENABLE_ADVANCED_DIAGNOSTICS != 0;

[[nodiscard]] constexpr bool advancedDiagnosticsEnabled() noexcept {
	return AdvancedDiagnosticsEnabled;
}

[[nodiscard]] Region regionOf(const void *ptr) noexcept;
[[nodiscard]] bool supports(Placement placement) noexcept;
[[nodiscard]] bool supports(Region region) noexcept;
[[nodiscard]] bool supports(Capability capabilities) noexcept;
[[nodiscard]] MemoryStats memoryStats(Region region) noexcept;
[[nodiscard]] AllocationDiagnostics allocationDiagnostics() noexcept;
[[nodiscard]] AllocationDiagnostics allocationDiagnostics(Placement placement) noexcept;
void resetAllocationDiagnostics() noexcept;

} // namespace Strata
