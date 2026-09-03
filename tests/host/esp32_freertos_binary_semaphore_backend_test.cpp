#include <strata/freertos/BinarySemaphore.h>
#include <esp_heap_caps.h>

#include <cassert>

int main() {
	using Strata::FreeRTOS::BinarySemaphore;
	using Strata::Placement;
	using Strata::Region;

	fake_heap_caps_reset();
	fake_semaphore_reset();
	auto semaphore = BinarySemaphore::create();
	assert(semaphore);
	assert(semaphore.controlPlacement() == Placement::Internal);
	assert(semaphore.controlRegion() == Region::Internal);
	assert(fake_heap_caps_internal_attempts == 1);
	assert(fake_heap_caps_external_attempts == 0);
	assert(fake_semaphore_binary_create_calls == 1);
	assert(!semaphore.tryTake());
	assert(semaphore.give());
	assert(semaphore.tryTake());
	semaphore.reset();
	assert(fake_heap_caps_allocations.empty());
	assert(fake_semaphore_delete_calls == 1);

	fake_heap_caps_reset();
	fake_semaphore_reset();
	fake_semaphore_fail_create = true;
	auto failedCreate = BinarySemaphore::create();
	assert(!failedCreate);
	assert(fake_semaphore_binary_create_calls == 1);
	assert(fake_heap_caps_allocations.empty());
	assert(fake_semaphore_delete_calls == 0);

	fake_heap_caps_reset();
	fake_semaphore_reset();
	fake_heap_caps_fail_internal = true;
	auto failedAllocation = BinarySemaphore::create();
	assert(!failedAllocation);
	assert(fake_heap_caps_internal_attempts == 1);
	assert(fake_heap_caps_external_attempts == 0);
	assert(fake_semaphore_binary_create_calls == 0);
	assert(fake_heap_caps_allocations.empty());
	assert(fake_semaphore_delete_calls == 0);
}
