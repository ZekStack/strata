#include <Strata.h>

#include <cassert>

int main() {
	static_assert(Strata::AdvancedDiagnosticsEnabled);
	assert(Strata::advancedDiagnosticsEnabled());

	Strata::resetAllocationDiagnostics();

	void *raw = Strata::allocate(32, Strata::Placement::Internal);
	assert(raw != nullptr);

	void *strictExternal = Strata::allocate(16, Strata::Placement::RequireExternal);
	assert(strictExternal == nullptr);

	void *invalid = Strata::allocate(0, Strata::Placement::Default);
	assert(invalid == nullptr);

	void *zeroed = Strata::calloc(2, 8, Strata::Placement::Internal);
	assert(zeroed != nullptr);

	raw = Strata::reallocate(raw, 64, Strata::Placement::Internal);
	assert(raw != nullptr);

	const auto total = Strata::allocationDiagnostics();
	assert(total.attempts == 5);
	assert(total.successes == 3);
	assert(total.failures == 2);
	assert(total.invalidRequests == 1);
	assert(total.preferredExternalFallbacks == 0);
	assert(total.requestedBytes == 128);
	assert(total.successfulBytes == 112);
	assert(total.failedBytes == 16);

	const auto internal = Strata::allocationDiagnostics(Strata::Placement::Internal);
	assert(internal.attempts == 3);
	assert(internal.successes == 3);
	assert(internal.failures == 0);
	assert(internal.requestedBytes == 112);
	assert(internal.successfulBytes == 112);

	const auto requiredExternal =
		Strata::allocationDiagnostics(Strata::Placement::RequireExternal);
	assert(requiredExternal.attempts == 1);
	assert(requiredExternal.failures == 1);
	assert(requiredExternal.failedBytes == 16);

	const auto defaultPlacement = Strata::allocationDiagnostics(Strata::Placement::Default);
	assert(defaultPlacement.attempts == 1);
	assert(defaultPlacement.failures == 1);
	assert(defaultPlacement.invalidRequests == 1);

	Strata::free(raw);
	Strata::free(zeroed);

	Strata::resetAllocationDiagnostics();
	assert(Strata::allocationDiagnostics().attempts == 0);
	return 0;
}
