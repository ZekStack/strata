#include <strata/freertos/BinarySemaphore.h>

#include <cassert>
#include <type_traits>
#include <utility>

int main() {
	using Strata::FreeRTOS::BinarySemaphore;

	static_assert(!std::is_copy_constructible_v<BinarySemaphore>);
	static_assert(!std::is_copy_assignable_v<BinarySemaphore>);
	static_assert(std::is_move_constructible_v<BinarySemaphore>);
	static_assert(std::is_move_assignable_v<BinarySemaphore>);

	fake_semaphore_reset();
	BinarySemaphore empty;
	assert(!empty);
	assert(empty.handle() == nullptr);
	assert(!empty.tryTake());
	assert(!empty.give());
	assert(!empty.giveFromISR());
	assert(!empty.takeFromISR());
	empty.reset();

	auto semaphore = BinarySemaphore::create();
	assert(semaphore);
	assert(semaphore.handle() != nullptr);
	assert(semaphore.controlPlacement() == Strata::Placement::Internal);
	assert(fake_semaphore_binary_create_calls == 1);
	assert(!semaphore.tryTake());
	assert(semaphore.give());
	assert(!semaphore.give());
	assert(semaphore.take(12));
	assert(fake_semaphore_last_ticks_to_wait == 12);
	assert(!semaphore.tryTake());

	BaseType_t taskWoken = pdFALSE;
	assert(semaphore.giveFromISR(&taskWoken));
	assert(taskWoken == pdTRUE);
	assert(fake_semaphore_give_from_isr_calls == 1);
	taskWoken = pdFALSE;
	assert(!semaphore.giveFromISR(&taskWoken));
	assert(taskWoken == pdFALSE);
	assert(fake_semaphore_give_from_isr_calls == 2);
	assert(semaphore.takeFromISR(&taskWoken));
	assert(taskWoken == pdTRUE);
	assert(fake_semaphore_take_from_isr_calls == 1);
	assert(!semaphore.takeFromISR());
	assert(fake_semaphore_take_from_isr_calls == 2);

	BinarySemaphore moved = std::move(semaphore);
	assert(!semaphore);
	assert(moved);

	auto replacement = BinarySemaphore::create();
	assert(replacement);
	replacement = std::move(moved);
	assert(!moved);
	assert(replacement);
	assert(fake_semaphore_delete_calls == 1);
	replacement.reset();
	assert(!replacement);
	assert(fake_semaphore_delete_calls == 2);
	replacement.reset();
	assert(fake_semaphore_delete_calls == 2);

	fake_semaphore_reset();
	fake_semaphore_fail_create = true;
	auto failed = BinarySemaphore::create();
	assert(!failed);
	assert(fake_semaphore_binary_create_calls == 1);
	assert(fake_semaphore_delete_calls == 0);
}
