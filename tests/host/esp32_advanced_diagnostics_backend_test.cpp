#include <Strata.h>
#include <esp_heap_caps.h>

#include <cassert>

int main() {
	static_assert(Strata::AdvancedDiagnosticsEnabled);
	fake_heap_caps_reset();
	Strata::resetAllocationDiagnostics();

	void *external = Strata::allocate(64, Strata::Placement::PreferExternal);
	assert(external != nullptr);
	assert(Strata::regionOf(external) == Strata::Region::External);

	fake_heap_caps_fail_external = true;
	void *fallback = Strata::allocate(32, Strata::Placement::PreferExternal);
	assert(fallback != nullptr);
	assert(Strata::regionOf(fallback) == Strata::Region::Internal);

	void *requiredExternal = Strata::allocate(16, Strata::Placement::RequireExternal);
	assert(requiredExternal == nullptr);

	void *invalid = Strata::allocate(Strata::AllocationRequest{
		.sizeBytes = 8,
		.placement = Strata::Placement::Internal,
		.alignment = 3,
		.capabilities = Strata::Capability::None,
	});
	assert(invalid == nullptr);

	void *zeroed = Strata::calloc(2, 16, Strata::Placement::PreferExternal);
	assert(zeroed != nullptr);
	assert(Strata::regionOf(zeroed) == Strata::Region::Internal);

	const auto total = Strata::allocationDiagnostics();
	assert(total.attempts == 5);
	assert(total.successes == 3);
	assert(total.failures == 2);
	assert(total.invalidRequests == 1);
	assert(total.preferredExternalFallbacks == 2);
	assert(total.requestedBytes == 152);
	assert(total.successfulBytes == 128);
	assert(total.failedBytes == 24);

	const auto preferred = Strata::allocationDiagnostics(Strata::Placement::PreferExternal);
	assert(preferred.attempts == 3);
	assert(preferred.successes == 3);
	assert(preferred.failures == 0);
	assert(preferred.preferredExternalFallbacks == 2);
	assert(preferred.requestedBytes == 128);
	assert(preferred.successfulBytes == 128);

	Strata::free(external);
	Strata::free(fallback);
	Strata::free(zeroed);
	return 0;
}
