#pragma once

#include <freertos/FreeRTOS.h>

#include <cstddef>

struct StaticSemaphore_t {
	bool recursive{false};
	std::size_t depth{0};
	bool deleted{false};
};

using SemaphoreHandle_t = StaticSemaphore_t *;

inline std::size_t fake_semaphore_mutex_create_calls = 0;
inline std::size_t fake_semaphore_recursive_create_calls = 0;
inline std::size_t fake_semaphore_delete_calls = 0;
inline std::size_t fake_semaphore_take_calls = 0;
inline std::size_t fake_semaphore_give_calls = 0;
inline std::size_t fake_semaphore_recursive_take_calls = 0;
inline std::size_t fake_semaphore_recursive_give_calls = 0;
inline bool fake_semaphore_fail_create = false;

inline void fake_semaphore_reset() {
	fake_semaphore_mutex_create_calls = 0;
	fake_semaphore_recursive_create_calls = 0;
	fake_semaphore_delete_calls = 0;
	fake_semaphore_take_calls = 0;
	fake_semaphore_give_calls = 0;
	fake_semaphore_recursive_take_calls = 0;
	fake_semaphore_recursive_give_calls = 0;
	fake_semaphore_fail_create = false;
}

inline SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *controlBlock) {
	++fake_semaphore_mutex_create_calls;
	if (controlBlock == nullptr || fake_semaphore_fail_create) {
		return nullptr;
	}
	controlBlock->recursive = false;
	controlBlock->depth = 0;
	controlBlock->deleted = false;
	return controlBlock;
}

inline SemaphoreHandle_t xSemaphoreCreateRecursiveMutexStatic(StaticSemaphore_t *controlBlock) {
	++fake_semaphore_recursive_create_calls;
	if (controlBlock == nullptr || fake_semaphore_fail_create) {
		return nullptr;
	}
	controlBlock->recursive = true;
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

inline BaseType_t xSemaphoreTake(SemaphoreHandle_t handle, TickType_t) {
	++fake_semaphore_take_calls;
	if (handle == nullptr || handle->deleted || handle->recursive || handle->depth != 0) {
		return pdFALSE;
	}
	handle->depth = 1;
	return pdTRUE;
}

inline BaseType_t xSemaphoreGive(SemaphoreHandle_t handle) {
	++fake_semaphore_give_calls;
	if (handle == nullptr || handle->deleted || handle->recursive || handle->depth == 0) {
		return pdFALSE;
	}
	handle->depth = 0;
	return pdTRUE;
}

inline BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t handle, TickType_t) {
	++fake_semaphore_recursive_take_calls;
	if (handle == nullptr || handle->deleted || !handle->recursive) {
		return pdFALSE;
	}
	++handle->depth;
	return pdTRUE;
}

inline BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t handle) {
	++fake_semaphore_recursive_give_calls;
	if (handle == nullptr || handle->deleted || !handle->recursive || handle->depth == 0) {
		return pdFALSE;
	}
	--handle->depth;
	return pdTRUE;
}
