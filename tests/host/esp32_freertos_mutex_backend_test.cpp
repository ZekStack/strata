#include <strata/freertos/Mutex.h>
#include <esp_heap_caps.h>

#include <cassert>

int main() {
	using Strata::FreeRTOS::Mutex;
	using Strata::FreeRTOS::RecursiveMutex;
	using Strata::Placement;
	using Strata::Region;

	fake_heap_caps_reset();
	fake_semaphore_reset();
	auto mutex = Mutex::create();
	assert(mutex);
	assert(mutex.controlPlacement() == Placement::Internal);
	assert(mutex.controlRegion() == Region::Internal);
	assert(fake_heap_caps_internal_attempts == 1);
	assert(fake_heap_caps_external_attempts == 0);
	assert(mutex.tryLock());
	mutex.unlock();
	mutex.reset();
	assert(fake_heap_caps_allocations.empty());
	assert(fake_semaphore_delete_calls == 1);

	fake_heap_caps_reset();
	fake_semaphore_reset();
	auto recursive = RecursiveMutex::create();
	assert(recursive);
	assert(recursive.controlPlacement() == Placement::Internal);
	assert(recursive.controlRegion() == Region::Internal);
	assert(fake_heap_caps_internal_attempts == 1);
	assert(fake_heap_caps_external_attempts == 0);
	assert(recursive.lock());
	assert(recursive.lock());
	recursive.unlock();
	recursive.unlock();
	recursive.reset();
	assert(fake_heap_caps_allocations.empty());
	assert(fake_semaphore_delete_calls == 1);

	fake_heap_caps_reset();
	fake_semaphore_reset();
	fake_semaphore_fail_create = true;
	auto failed = Mutex::create();
	assert(!failed);
	assert(fake_heap_caps_allocations.empty());
	assert(fake_semaphore_delete_calls == 0);
}
