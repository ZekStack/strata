#include <strata/freertos/Queue.h>
#include <esp_heap_caps.h>

#include <cassert>

int main() {
	using Strata::FreeRTOS::Queue;
	using Strata::FreeRTOS::QueueConfig;
	using Strata::FreeRTOS::QueueUsage;
	using Strata::Placement;
	using Strata::Region;

	fake_heap_caps_reset();
	fake_queue_reset();
	auto external = Queue<int>::create(QueueConfig{
		.length = 8,
		.storagePlacement = Placement::PreferExternal,
		.usage = QueueUsage::TaskOnly,
	});
	assert(external);
	assert(external.storageRegion() == Region::External);
	assert(external.controlRegion() == Region::Internal);
	assert(external.controlPlacement() == Placement::Internal);
	external.reset();
	assert(fake_heap_caps_allocations.empty());

	fake_heap_caps_reset();
	fake_queue_reset();
	fake_heap_caps_fail_external = true;
	auto fallback = Queue<int>::create(QueueConfig{
		.length = 8,
		.storagePlacement = Placement::PreferExternal,
		.usage = QueueUsage::TaskOnly,
	});
	assert(fallback);
	assert(fallback.storageRegion() == Region::Internal);
	assert(fallback.storagePlacement() == Placement::PreferExternal);
	fallback.reset();
	assert(fake_heap_caps_allocations.empty());

	fake_heap_caps_reset();
	fake_queue_reset();
	fake_heap_caps_external_total = 0;
	auto strict = Queue<int>::create(QueueConfig{
		.length = 8,
		.storagePlacement = Placement::RequireExternal,
		.usage = QueueUsage::TaskOnly,
	});
	assert(!strict);
	assert(fake_queue_create_calls == 0);
	assert(fake_heap_caps_allocations.empty());

	fake_heap_caps_reset();
	fake_queue_reset();
	auto isrQueue = Queue<int>::create(QueueConfig{
		.length = 4,
		.storagePlacement = Placement::Internal,
		.usage = QueueUsage::IsrAccessible,
	});
	assert(isrQueue);
	assert(isrQueue.storageRegion() == Region::Internal);
	assert(isrQueue.controlRegion() == Region::Internal);
	int value = 42;
	assert(isrQueue.sendFromISR(value));
	int received = 0;
	assert(isrQueue.receiveFromISR(received));
	assert(received == 42);
	isrQueue.reset();
	assert(fake_heap_caps_allocations.empty());

	fake_heap_caps_reset();
	fake_queue_reset();
	auto rejectedIsrExternal = Queue<int>::create(QueueConfig{
		.length = 4,
		.storagePlacement = Placement::PreferExternal,
		.usage = QueueUsage::IsrAccessible,
	});
	assert(!rejectedIsrExternal);
	assert(fake_queue_create_calls == 0);
	assert(fake_heap_caps_allocations.empty());
}
