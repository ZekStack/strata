#include "Allocation.h"

#include "internal/DiagnosticsState.h"
#include "internal/Platform.h"

#include <limits>

namespace Strata {
namespace {

[[nodiscard]] constexpr bool isValidAlignment(std::size_t alignment) noexcept {
	return alignment != 0 && (alignment & (alignment - 1)) == 0;
}

[[nodiscard]] constexpr bool multiplyWouldOverflow(std::size_t lhs, std::size_t rhs) noexcept {
	return rhs != 0 && lhs > (std::numeric_limits<std::size_t>::max() / rhs);
}

#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
void recordAttempt(std::size_t sizeBytes, Placement placement) noexcept {
	Internal::recordAllocationAttempt(sizeBytes, placement);
}

void recordResult(
	std::size_t sizeBytes,
	Placement placement,
	void *ptr,
	bool invalidRequest = false) noexcept {
	Internal::recordAllocationResult(
		sizeBytes,
		placement,
		ptr != nullptr,
		invalidRequest,
		ptr != nullptr ? Internal::regionOf(ptr) : Region::Unknown);
}
#endif

} // namespace

void *allocate(std::size_t sizeBytes, Placement placement) noexcept {
	return allocate(AllocationRequest{
		.sizeBytes = sizeBytes,
		.placement = placement,
		.alignment = DefaultAlignment,
		.capabilities = Capability::None,
	});
}

void *allocate(const AllocationRequest &request) noexcept {
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
	recordAttempt(request.sizeBytes, request.placement);
#endif

	if (
		request.sizeBytes == 0 ||
		!isValidAlignment(request.alignment) ||
		!validCapabilities(request.capabilities)) {
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
		recordResult(request.sizeBytes, request.placement, nullptr, true);
#endif
		return nullptr;
	}

	auto *ptr = Internal::allocate(
		request.sizeBytes,
		request.alignment,
		request.placement,
		request.capabilities);
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
	recordResult(request.sizeBytes, request.placement, ptr);
#endif
	return ptr;
}

void *calloc(std::size_t count, std::size_t sizeBytes, Placement placement) noexcept {
	const bool overflow = multiplyWouldOverflow(count, sizeBytes);
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
	const std::size_t requestedBytes = overflow ? 0 : count * sizeBytes;
	recordAttempt(requestedBytes, placement);
#endif

	if (count == 0 || sizeBytes == 0 || overflow) {
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
		recordResult(requestedBytes, placement, nullptr, true);
#endif
		return nullptr;
	}

	auto *ptr = Internal::calloc(count, sizeBytes, placement);
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
	recordResult(requestedBytes, placement, ptr);
#endif
	return ptr;
}

void *reallocate(void *ptr, std::size_t newSizeBytes, Placement placement) noexcept {
	if (newSizeBytes == 0) {
		Internal::free(ptr);
		return nullptr;
	}

	if (ptr == nullptr) {
		return allocate(newSizeBytes, placement);
	}

#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
	recordAttempt(newSizeBytes, placement);
#endif
	auto *resized = Internal::reallocate(ptr, newSizeBytes, placement);
#if STRATA_ENABLE_ADVANCED_DIAGNOSTICS
	recordResult(newSizeBytes, placement, resized);
#endif
	return resized;
}

void free(void *ptr) noexcept {
	Internal::free(ptr);
}

} // namespace Strata
