#include "Diagnostics.h"

#include "internal/DiagnosticsState.h"
#include "internal/Platform.h"

#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
#include <array>
#include <atomic>
#endif

namespace Strata {

#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
namespace {

struct AtomicAllocationDiagnostics {
	std::atomic_size_t attempts{0};
	std::atomic_size_t successes{0};
	std::atomic_size_t failures{0};
	std::atomic_size_t invalidRequests{0};
	std::atomic_size_t preferredExternalFallbacks{0};
	std::atomic_size_t requestedBytes{0};
	std::atomic_size_t successfulBytes{0};
	std::atomic_size_t failedBytes{0};
};

struct AdvancedDiagnosticsState {
	AtomicAllocationDiagnostics total{};
	std::array<AtomicAllocationDiagnostics, 4> placements{};
};

AdvancedDiagnosticsState diagnosticsState{};

[[nodiscard]] constexpr std::size_t placementIndex(Placement placement) noexcept {
	switch (placement) {
		case Placement::Default:
			return 0;
		case Placement::Internal:
			return 1;
		case Placement::PreferExternal:
			return 2;
		case Placement::RequireExternal:
			return 3;
	}
	return 0;
}

void recordAttempt(AtomicAllocationDiagnostics &stats, std::size_t sizeBytes) noexcept {
	stats.attempts.fetch_add(1, std::memory_order_relaxed);
	stats.requestedBytes.fetch_add(sizeBytes, std::memory_order_relaxed);
}

void recordResult(
	AtomicAllocationDiagnostics &stats,
	std::size_t sizeBytes,
	bool success,
	bool invalidRequest,
	bool preferredExternalFallback) noexcept {
	if (success) {
		stats.successes.fetch_add(1, std::memory_order_relaxed);
		stats.successfulBytes.fetch_add(sizeBytes, std::memory_order_relaxed);
		if (preferredExternalFallback) {
			stats.preferredExternalFallbacks.fetch_add(1, std::memory_order_relaxed);
		}
		return;
	}

	stats.failures.fetch_add(1, std::memory_order_relaxed);
	stats.failedBytes.fetch_add(sizeBytes, std::memory_order_relaxed);
	if (invalidRequest) {
		stats.invalidRequests.fetch_add(1, std::memory_order_relaxed);
	}
}

[[nodiscard]] AllocationDiagnostics snapshot(const AtomicAllocationDiagnostics &stats) noexcept {
	return AllocationDiagnostics{
		.attempts = stats.attempts.load(std::memory_order_relaxed),
		.successes = stats.successes.load(std::memory_order_relaxed),
		.failures = stats.failures.load(std::memory_order_relaxed),
		.invalidRequests = stats.invalidRequests.load(std::memory_order_relaxed),
		.preferredExternalFallbacks =
			stats.preferredExternalFallbacks.load(std::memory_order_relaxed),
		.requestedBytes = stats.requestedBytes.load(std::memory_order_relaxed),
		.successfulBytes = stats.successfulBytes.load(std::memory_order_relaxed),
		.failedBytes = stats.failedBytes.load(std::memory_order_relaxed),
	};
}

void reset(AtomicAllocationDiagnostics &stats) noexcept {
	stats.attempts.store(0, std::memory_order_relaxed);
	stats.successes.store(0, std::memory_order_relaxed);
	stats.failures.store(0, std::memory_order_relaxed);
	stats.invalidRequests.store(0, std::memory_order_relaxed);
	stats.preferredExternalFallbacks.store(0, std::memory_order_relaxed);
	stats.requestedBytes.store(0, std::memory_order_relaxed);
	stats.successfulBytes.store(0, std::memory_order_relaxed);
	stats.failedBytes.store(0, std::memory_order_relaxed);
}

} // namespace
#endif

Region regionOf(const void *ptr) noexcept {
	if (ptr == nullptr) {
		return Region::Unknown;
	}
	return Internal::regionOf(ptr);
}

bool supports(Placement placement) noexcept {
	return validPlacement(placement) && Internal::supports(placement);
}

bool supports(Region region) noexcept {
	return Internal::supports(region);
}

bool supports(Capability capabilities) noexcept {
	return validCapabilities(capabilities) && Internal::supports(capabilities);
}

MemoryStats memoryStats(Region region) noexcept {
	return Internal::memoryStats(region);
}

AllocationDiagnostics allocationDiagnostics() noexcept {
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
	return snapshot(diagnosticsState.total);
#else
	return {};
#endif
}

AllocationDiagnostics allocationDiagnostics(Placement placement) noexcept {
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
	if (!validPlacement(placement)) {
		return {};
	}
	return snapshot(diagnosticsState.placements[placementIndex(placement)]);
#else
	(void)placement;
	return {};
#endif
}

void resetAllocationDiagnostics() noexcept {
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
	reset(diagnosticsState.total);
	for (auto &placement : diagnosticsState.placements) {
		reset(placement);
	}
#endif
}

#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
namespace Internal {

void recordAllocationAttempt(std::size_t sizeBytes, Placement placement) noexcept {
	recordAttempt(diagnosticsState.total, sizeBytes);
	if (validPlacement(placement)) {
		recordAttempt(diagnosticsState.placements[placementIndex(placement)], sizeBytes);
	}
}

void recordAllocationResult(
	std::size_t sizeBytes,
	Placement placement,
	bool success,
	bool invalidRequest,
	Region region) noexcept {
	const bool preferredExternalFallback =
		success && placement == Placement::PreferExternal && region == Region::Internal;

	recordResult(
		diagnosticsState.total,
		sizeBytes,
		success,
		invalidRequest,
		preferredExternalFallback);
	if (validPlacement(placement)) {
		recordResult(
			diagnosticsState.placements[placementIndex(placement)],
			sizeBytes,
			success,
			invalidRequest,
			preferredExternalFallback);
	}
}

} // namespace Internal
#endif

} // namespace Strata
