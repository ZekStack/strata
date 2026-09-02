#include <strata/freertos/Mutex.h>

#include <cassert>
#include <type_traits>
#include <utility>

int main() {
	using Strata::FreeRTOS::Mutex;
	using Strata::FreeRTOS::RecursiveMutex;

	static_assert(!std::is_copy_constructible_v<Mutex>);
	static_assert(std::is_move_constructible_v<Mutex>);
	static_assert(std::is_move_assignable_v<Mutex>);
	static_assert(!std::is_copy_constructible_v<RecursiveMutex>);
	static_assert(std::is_move_constructible_v<RecursiveMutex>);
	static_assert(std::is_move_assignable_v<RecursiveMutex>);

	fake_semaphore_reset();
	Mutex empty;
	assert(!empty);
	assert(empty.handle() == nullptr);
	assert(!empty.tryLock());
	empty.unlock();
	empty.reset();

	auto mutex = Mutex::create();
	assert(mutex);
	assert(mutex.handle() != nullptr);
	assert(mutex.controlPlacement() == Strata::Placement::Internal);
	assert(fake_semaphore_mutex_create_calls == 1);
	assert(mutex.tryLock());
	assert(!mutex.tryLock());
	mutex.unlock();
	assert(mutex.lock(12));
	mutex.unlock();

	Mutex moved = std::move(mutex);
	assert(!mutex);
	assert(moved);

	auto replacement = Mutex::create();
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
	auto recursive = RecursiveMutex::create();
	assert(recursive);
	assert(recursive.controlPlacement() == Strata::Placement::Internal);
	assert(fake_semaphore_recursive_create_calls == 1);
	assert(recursive.lock());
	assert(recursive.tryLock());
	recursive.unlock();
	recursive.unlock();
	assert(fake_semaphore_recursive_take_calls == 2);
	assert(fake_semaphore_recursive_give_calls == 2);
	recursive.reset();
	assert(fake_semaphore_delete_calls == 1);

	fake_semaphore_reset();
	fake_semaphore_fail_create = true;
	auto failedMutex = Mutex::create();
	assert(!failedMutex);
	assert(fake_semaphore_mutex_create_calls == 1);
	assert(fake_semaphore_delete_calls == 0);
	auto failedRecursive = RecursiveMutex::create();
	assert(!failedRecursive);
	assert(fake_semaphore_recursive_create_calls == 1);
	assert(fake_semaphore_delete_calls == 0);
}
