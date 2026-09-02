#include <Strata.h>
#include <esp_heap_caps.h>

#include <cassert>

int main() {
	fake_heap_caps_reset();

	assert(Strata::supports(Strata::Placement::Default));
	assert(Strata::supports(Strata::Placement::Internal));
	assert(Strata::supports(Strata::Placement::PreferExternal));
	assert(Strata::supports(Strata::Placement::RequireExternal));
	assert(!Strata::supports(Strata::Region::Unknown));
	assert(Strata::supports(Strata::Region::Internal));
	assert(Strata::supports(Strata::Region::External));

	void *internalPtr = Strata::allocate(64, Strata::Placement::Internal);
	void *externalPtr = Strata::allocate(64, Strata::Placement::RequireExternal);
	assert(internalPtr != nullptr);
	assert(externalPtr != nullptr);
	assert(Strata::regionOf(internalPtr) == Strata::Region::Internal);
	assert(Strata::regionOf(externalPtr) == Strata::Region::External);

	int foreign = 0;
	assert(Strata::regionOf(&foreign) == Strata::Region::Unknown);
	assert(Strata::regionOf(nullptr) == Strata::Region::Unknown);

	const auto internal = Strata::memoryStats(Strata::Region::Internal);
	assert(internal.totalBytes == fake_heap_caps_internal_total);
	assert(internal.freeBytes == fake_heap_caps_internal_free);
	assert(internal.minimumFreeBytes == fake_heap_caps_internal_minimum_free);
	assert(internal.largestFreeBlockBytes == fake_heap_caps_internal_largest);
	assert(internal.usedBytes == fake_heap_caps_internal_total - fake_heap_caps_internal_free);
	assert(
		internal.peakUsedBytes ==
		fake_heap_caps_internal_total - fake_heap_caps_internal_minimum_free);

	const auto external = Strata::memoryStats(Strata::Region::External);
	assert(external.totalBytes == fake_heap_caps_external_total);
	assert(external.freeBytes == fake_heap_caps_external_free);
	assert(external.minimumFreeBytes == fake_heap_caps_external_minimum_free);
	assert(external.largestFreeBlockBytes == fake_heap_caps_external_largest);
	assert(external.usedBytes == fake_heap_caps_external_total - fake_heap_caps_external_free);
	assert(
		external.peakUsedBytes ==
		fake_heap_caps_external_total - fake_heap_caps_external_minimum_free);

	fake_heap_caps_external_total = 0;
	assert(!Strata::supports(Strata::Placement::RequireExternal));
	assert(!Strata::supports(Strata::Region::External));
	const auto unavailable = Strata::memoryStats(Strata::Region::External);
	assert(!unavailable.totalBytes.has_value());
	assert(!unavailable.freeBytes.has_value());
	assert(!unavailable.minimumFreeBytes.has_value());
	assert(!unavailable.largestFreeBlockBytes.has_value());
	assert(!unavailable.usedBytes.has_value());
	assert(!unavailable.peakUsedBytes.has_value());

	Strata::free(internalPtr);
	Strata::free(externalPtr);
	return 0;
}
