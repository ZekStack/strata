#include <strata/freertos/CountingSemaphore.h>

#include <cassert>
#include <type_traits>
#include <utility>

int main() {
	using Strata::FreeRTOS::CountingSemaphore;

	static_assert(!std::is_copy_constructible_v<CountingSemaphore>);
	static_assert(!std::is_copy_assignable_v<CountingSemaphore>);
	static_assert(std::is_move_constructible_v<CountingSemaphore>);
	static_assert(std::is_move_assignable_v<CountingSemaphore>);

	fake_semaphore_reset();
	CountingSemaphore empty;
	assert(!empty);
	assert(empty.handle() == nullptr);
	assert(empty.maxCount() == 0);
	assert(!empty.tryTake());
	assert(!empty.give());
	empty.reset();

	auto semaphore = CountingSemaphore::create(3, 2);
	assert(semaphore);
	assert(semaphore.handle() != nullptr);
	assert(semaphore.maxCount() == 3);
	assert(semaphore.controlPlacement() == Strata::Placement::Internal);
	assert(fake_semaphore_counting_create_calls == 1);
	assert(semaphore.tryTake());
	assert(semaphore.take(12));
	assert(fake_semaphore_last_ticks_to_wait == 12);
	assert(!semaphore.tryTake());
	assert(semaphore.give());
	assert(semaphore.give());
	assert(semaphore.give());
	assert(!semaphore.give());

	BaseType_t taskWoken = pdFALSE;
	assert(semaphore.takeFromISR(&taskWoken));
	assert(taskWoken == pdTRUE);
	taskWoken = pdFALSE;
	assert(semaphore.giveFromISR(&taskWoken));
	assert(taskWoken == pdTRUE);

	CountingSemaphore moved = std::move(semaphore);
	assert(!semaphore);
	assert(moved);
	assert(moved.maxCount() == 3);

	auto replacement = CountingSemaphore::create(1, 0);
	assert(replacement);
	replacement = std::move(moved);
	assert(!moved);
	assert(replacement);
	assert(replacement.maxCount() == 3);
	assert(fake_semaphore_delete_calls == 1);
	replacement.reset();
	assert(!replacement);
	assert(fake_semaphore_delete_calls == 2);

	fake_semaphore_reset();
	assert(!CountingSemaphore::create(0, 0));
	assert(!CountingSemaphore::create(2, 3));
	assert(fake_semaphore_counting_create_calls == 0);

	fake_semaphore_fail_create = true;
	auto failed = CountingSemaphore::create(2, 1);
	assert(!failed);
	assert(fake_semaphore_counting_create_calls == 1);
	assert(fake_semaphore_delete_calls == 0);
}
