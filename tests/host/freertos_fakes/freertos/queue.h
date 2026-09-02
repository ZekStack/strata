#pragma once

#include <freertos/FreeRTOS.h>

using QueueHandle_t = StaticQueue_t *;

inline std::size_t fake_queue_create_calls = 0;
inline std::size_t fake_queue_delete_calls = 0;
inline std::size_t fake_queue_send_calls = 0;
inline std::size_t fake_queue_receive_calls = 0;
inline std::size_t fake_queue_send_from_isr_calls = 0;
inline std::size_t fake_queue_receive_from_isr_calls = 0;

inline void fake_queue_reset() {
	fake_queue_create_calls = 0;
	fake_queue_delete_calls = 0;
	fake_queue_send_calls = 0;
	fake_queue_receive_calls = 0;
	fake_queue_send_from_isr_calls = 0;
	fake_queue_receive_from_isr_calls = 0;
}

inline void fake_queue_copy(unsigned char *destination, const unsigned char *source, std::size_t size) {
	for (std::size_t i = 0; i < size; ++i) {
		destination[i] = source[i];
	}
}

inline QueueHandle_t xQueueCreateStatic(
	UBaseType_t length,
	UBaseType_t itemSize,
	std::uint8_t *storage,
	StaticQueue_t *controlBlock) {
	++fake_queue_create_calls;
	if (length == 0 || itemSize == 0 || storage == nullptr || controlBlock == nullptr) {
		return nullptr;
	}
	controlBlock->storage = storage;
	controlBlock->length = length;
	controlBlock->itemSize = itemSize;
	controlBlock->head = 0;
	controlBlock->tail = 0;
	controlBlock->count = 0;
	controlBlock->deleted = false;
	return controlBlock;
}

inline void vQueueDelete(QueueHandle_t handle) {
	if (handle != nullptr) {
		++fake_queue_delete_calls;
		handle->deleted = true;
	}
}

inline BaseType_t xQueueSend(QueueHandle_t handle, const void *item, TickType_t) {
	++fake_queue_send_calls;
	if (handle == nullptr || item == nullptr || handle->deleted || handle->count >= handle->length) {
		return pdFALSE;
	}
	auto *destination = handle->storage + (handle->tail * handle->itemSize);
	fake_queue_copy(destination, static_cast<const unsigned char *>(item), handle->itemSize);
	handle->tail = (handle->tail + 1) % handle->length;
	++handle->count;
	return pdTRUE;
}

inline BaseType_t xQueueReceive(QueueHandle_t handle, void *item, TickType_t) {
	++fake_queue_receive_calls;
	if (handle == nullptr || item == nullptr || handle->deleted || handle->count == 0) {
		return pdFALSE;
	}
	const auto *source = handle->storage + (handle->head * handle->itemSize);
	fake_queue_copy(static_cast<unsigned char *>(item), source, handle->itemSize);
	handle->head = (handle->head + 1) % handle->length;
	--handle->count;
	return pdTRUE;
}

inline BaseType_t xQueueSendFromISR(
	QueueHandle_t handle,
	const void *item,
	BaseType_t *higherPriorityTaskWoken) {
	++fake_queue_send_from_isr_calls;
	if (higherPriorityTaskWoken != nullptr) {
		*higherPriorityTaskWoken = pdFALSE;
	}
	return xQueueSend(handle, item, 0);
}

inline BaseType_t xQueueReceiveFromISR(
	QueueHandle_t handle,
	void *item,
	BaseType_t *higherPriorityTaskWoken) {
	++fake_queue_receive_from_isr_calls;
	if (higherPriorityTaskWoken != nullptr) {
		*higherPriorityTaskWoken = pdFALSE;
	}
	return xQueueReceive(handle, item, 0);
}
