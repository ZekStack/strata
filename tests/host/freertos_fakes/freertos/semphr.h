#pragma once

#include <freertos/FreeRTOS.h>

#include <cstddef>

enum class FakeSemaphoreKind {
	Mutex,
	RecursiveMutex,
	Binary,
};

struct StaticSemaphore_t {
	FakeSemaphoreKind kind{FakeSemaphoreKind::Mutex};
	std::size_t depth{0};
	bool deleted{false};
};

using SemaphoreHandle_t = StaticSemaphore_t *;

inline std::size_t fake_semaphore_mutex_create_calls = 0;
inline std::size_t fake_semaphore_recursive_create_calls = 0;
inline std::size_t fake_semaphore_binary_create_calls = 0;
inline std::size_t fake_semaphore_delete_calls = 0;
inline std::size_t fake_semaphore_take_calls = 0;
inline std::size_t fake_semaphore_give_calls = 0;
inline std::size_t fake_semaphore_take_from_isr_calls = 0;
inline std::size_t fake_semaphore_give_from_isr_calls = 0;
inline std::size_t fake_semaphore_recursive_take_calls = 0;
inline std::size_t fake_semaphore_recursive_give_calls = 0;
inline TickType_t fake_semaphore_last_ticks_to_wait = 0;
inline bool fake_semaphore_fail_create = false;

inline void fake_semaphore_reset() {
	fake_semaphore_mutex_create_calls = 0;
	fake_semaphore_recursive_create_calls = 0;
	fake_semaphore_binary_create_calls = 0;
	fake_semaphore_delete_calls = 0;
	fake_semaphore_take_calls = 0;
	fake_semaphore_give_calls = 0;
	fake_semaphore_take_from_isr_calls = 0;
	fake_semaphore_give_from_isr_calls = 0;
	fake_semaphore_recursive_take_calls = 0;
	fake_semaphore_recursive_give_calls = 0;
	fake_semaphore_last_ticks_to_wait = 0;
	fake_semaphore_fail_create = false;
}

inline SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *controlBlock) {
	++fake_semaphore_mutex_create_calls;
	if (controlBlock == nullptr || fake_semaphore_fail_create) {
		return nullptr;
	}
	controlBlock->kind = FakeSemaphoreKind::Mutex;
	controlBlock->depth = 0;
	controlBlock->deleted = false;
	return controlBlock;
}

inline SemaphoreHandle_t xSemaphoreCreateRecursiveMutexStatic(StaticSemaphore_t *controlBlock) {
	++fake_semaphore_recursive_create_calls;
	if (controlBlock == nullptr || fake_semaphore_fail_create) {
		return nullptr;
	}
	controlBlock->kind = FakeSemaphoreKind::RecursiveMutex;
	controlBlock->depth = 0;
	controlBlock->deleted = false;
	return controlBlock;
}

inline SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t *controlBlock) {
	++fake_semaphore_binary_create_calls;
	if (controlBlock == nullptr || fake_semaphore_fail_create) {
		return nullptr;
	}
	controlBlock->kind = FakeSemaphoreKind::Binary;
	controlBlock->depth = 0;
	controlBlock->deleted = false;
	return controlBlock;
}

inline void vSemaphoreDelete(SemaphoreHandle_t handle) {
	if (handle != nullptr) {
		++fake_semaphore_delete_calls;
		handle->deleted = true;
	}
}

inline BaseType_t xSemaphoreTake(SemaphoreHandle_t handle, TickType_t ticksToWait) {
	++fake_semaphore_take_calls;
	fake_semaphore_last_ticks_to_wait = ticksToWait;
	if (handle == nullptr || handle->deleted || handle->kind == FakeSemaphoreKind::RecursiveMutex) {
		return pdFALSE;
	}
	if (handle->kind == FakeSemaphoreKind::Mutex) {
		if (handle->depth != 0) {
			return pdFALSE;
		}
		handle->depth = 1;
		return pdTRUE;
	}
	if (handle->depth == 0) {
		return pdFALSE;
	}
	handle->depth = 0;
	return pdTRUE;
}

inline BaseType_t xSemaphoreGive(SemaphoreHandle_t handle) {
	++fake_semaphore_give_calls;
	if (handle == nullptr || handle->deleted || handle->kind == FakeSemaphoreKind::RecursiveMutex) {
		return pdFALSE;
	}
	if (handle->kind == FakeSemaphoreKind::Mutex) {
		if (handle->depth == 0) {
			return pdFALSE;
		}
		handle->depth = 0;
		return pdTRUE;
	}
	if (handle->depth != 0) {
		return pdFALSE;
	}
	handle->depth = 1;
	return pdTRUE;
}

inline BaseType_t xSemaphoreTakeFromISR(
	SemaphoreHandle_t handle,
	BaseType_t *higherPriorityTaskWoken) {
	++fake_semaphore_take_from_isr_calls;
	if (handle == nullptr || handle->deleted || handle->kind != FakeSemaphoreKind::Binary ||
		handle->depth == 0) {
		return pdFALSE;
	}
	handle->depth = 0;
	if (higherPriorityTaskWoken != nullptr) {
		*higherPriorityTaskWoken = pdTRUE;
	}
	return pdTRUE;
}

inline BaseType_t xSemaphoreGiveFromISR(
	SemaphoreHandle_t handle,
	BaseType_t *higherPriorityTaskWoken) {
	++fake_semaphore_give_from_isr_calls;
	if (handle == nullptr || handle->deleted || handle->kind != FakeSemaphoreKind::Binary ||
		handle->depth != 0) {
		return pdFALSE;
	}
	handle->depth = 1;
	if (higherPriorityTaskWoken != nullptr) {
		*higherPriorityTaskWoken = pdTRUE;
	}
	return pdTRUE;
}

inline BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t handle, TickType_t) {
	++fake_semaphore_recursive_take_calls;
	if (handle == nullptr || handle->deleted || handle->kind != FakeSemaphoreKind::RecursiveMutex) {
		return pdFALSE;
	}
	++handle->depth;
	return pdTRUE;
}

inline BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t handle) {
	++fake_semaphore_recursive_give_calls;
	if (handle == nullptr || handle->deleted || handle->kind != FakeSemaphoreKind::RecursiveMutex ||
		handle->depth == 0) {
		return pdFALSE;
	}
	--handle->depth;
	return pdTRUE;
}
