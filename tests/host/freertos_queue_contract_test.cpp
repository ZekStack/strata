#include <strata/freertos/Queue.h>

#include <cassert>
#include <type_traits>

int main() {
	using Strata::FreeRTOS::Queue;
	using Strata::FreeRTOS::QueueConfig;
	using Strata::FreeRTOS::QueueUsage;

	static_assert(!std::is_copy_constructible_v<Queue<int>>);
	static_assert(std::is_move_constructible_v<Queue<int>>);

	fake_queue_reset();
	auto queue = Queue<int>::create(QueueConfig{
		.length = 3,
		.storagePlacement = Strata::Placement::Internal,
		.usage = QueueUsage::TaskOnly,
	});
	assert(queue);
	assert(queue.handle() != nullptr);
	assert(queue.length() == 3);
	assert(queue.storageBytes() == 3 * sizeof(int));
	assert(queue.storagePlacement() == Strata::Placement::Internal);
	assert(queue.controlPlacement() == Strata::Placement::Internal);
	assert(queue.usage() == QueueUsage::TaskOnly);

	int value = 17;
	assert(queue.send(value));
	int received = 0;
	assert(queue.receive(received));
	assert(received == 17);
	assert(!queue.sendFromISR(value));

	Queue<int> moved = std::move(queue);
	assert(!queue);
	assert(moved);
	moved.reset();
	assert(fake_queue_delete_calls == 1);

	fake_queue_reset();
	auto isrQueue = Queue<int>::create(QueueConfig{
		.length = 2,
		.storagePlacement = Strata::Placement::Internal,
		.usage = QueueUsage::IsrAccessible,
	});
	assert(isrQueue);
	BaseType_t woken = pdFALSE;
	assert(isrQueue.sendFromISR(value, &woken));
	received = 0;
	assert(isrQueue.receiveFromISR(received, &woken));
	assert(received == 17);
	assert(fake_queue_send_from_isr_calls == 1);
	assert(fake_queue_receive_from_isr_calls == 1);

	auto invalidIsrQueue = Queue<int>::create(QueueConfig{
		.length = 2,
		.storagePlacement = Strata::Placement::PreferExternal,
		.usage = QueueUsage::IsrAccessible,
	});
	assert(!invalidIsrQueue);

	auto zeroLength = Queue<int>::create(QueueConfig{});
	assert(!zeroLength);
}
