#include "Platform.h"

#if defined(ESP32)

#include <esp_heap_caps.h>
#include <esp_memory_utils.h>

namespace Strata::Internal {
namespace {

constexpr std::uint32_t DefaultCaps = MALLOC_CAP_8BIT;
constexpr std::uint32_t InternalCaps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
constexpr std::uint32_t ExternalCaps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;

[[nodiscard]] constexpr std::uint32_t capabilityCaps(Capability capabilities) noexcept {
	std::uint32_t caps = 0;
	if (hasCapability(capabilities, Capability::Dma)) {
		caps |= MALLOC_CAP_DMA;
	}
	if (hasCapability(capabilities, Capability::Executable)) {
		caps |= MALLOC_CAP_EXEC;
	}
	return caps;
}

[[nodiscard]] constexpr std::uint32_t byteCaps(Capability capabilities) noexcept {
	return hasCapability(capabilities, Capability::Executable) ? 0U : MALLOC_CAP_8BIT;
}

[[nodiscard]] constexpr std::uint32_t placementCaps(
	Placement placement,
	Capability capabilities) noexcept {
	const auto required = capabilityCaps(capabilities) | byteCaps(capabilities);
	switch (placement) {
		case Placement::Default:
			return required;
		case Placement::Internal:
			return MALLOC_CAP_INTERNAL | required;
		case Placement::PreferExternal:
		case Placement::RequireExternal:
			return MALLOC_CAP_SPIRAM | required;
	}
	return required;
}

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

	const auto totalBytes = heap_caps_get_total_size(caps);
	const auto freeBytes = heap_caps_get_free_size(caps);
	const auto minimumFreeBytes = heap_caps_get_minimum_free_size(caps);
	const auto largestFreeBlockBytes = heap_caps_get_largest_free_block(caps);

	return MemoryStats{
		.totalBytes = totalBytes,
		.freeBytes = freeBytes,
		.minimumFreeBytes = minimumFreeBytes,
		.largestFreeBlockBytes = largestFreeBlockBytes,
		.usedBytes = freeBytes <= totalBytes
			? std::optional<std::size_t>{totalBytes - freeBytes}
			: std::nullopt,
		.peakUsedBytes = minimumFreeBytes <= totalBytes
			? std::optional<std::size_t>{totalBytes - minimumFreeBytes}
			: std::nullopt,
	};
}

} // namespace

PlatformKind platformKind() noexcept {
	return PlatformKind::Esp32;
}

const char *platformName() noexcept {
	return "esp32";
}

void *allocate(
	std::size_t sizeBytes,
	std::size_t alignment,
	Placement placement,
	Capability capabilities) noexcept {
	if (placement == Placement::PreferExternal) {
		const auto externalCaps = placementCaps(Placement::RequireExternal, capabilities);
		auto *ptr = allocateWithCaps(sizeBytes, alignment, externalCaps);
		if (ptr != nullptr) {
			return ptr;
		}
		return allocateWithCaps(
			sizeBytes,
			alignment,
			placementCaps(Placement::Internal, capabilities));
	}

	return allocateWithCaps(sizeBytes, alignment, placementCaps(placement, capabilities));
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
			return resized != nullptr
				? resized
				: reallocateWithCaps(ptr, newSizeBytes, InternalCaps);
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

bool supports(Capability capabilities) noexcept {
	if (capabilities == Capability::None) {
		return true;
	}
	return capsAvailable(capabilityCaps(capabilities) | byteCaps(capabilities));
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
