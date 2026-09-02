#pragma once

#include "../Allocation.h"
#include "../Diagnostics.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
}

#if configSUPPORT_STATIC_ALLOCATION != 1
#error "Strata FreeRTOS queue integration requires configSUPPORT_STATIC_ALLOCATION == 1"
#endif

namespace Strata::FreeRTOS {

enum class QueueUsage : std::uint8_t {
	TaskOnly,
	IsrAccessible,
};

struct QueueConfig {
	std::size_t length{0};
	Placement storagePlacement{Placement::Internal};
	QueueUsage usage{QueueUsage::TaskOnly};
};

template <typename T>
requires(std::is_object_v<T> && !std::is_array_v<T> && std::is_trivially_copyable_v<T>)
class Queue {
public:
	using value_type = T;
	using Handle = QueueHandle_t;

	Queue() noexcept = default;

	~Queue() noexcept {
		reset();
	}

	Queue(const Queue &) = delete;
	Queue &operator=(const Queue &) = delete;

	Queue(Queue &&other) noexcept {
		moveFrom(other);
	}

	Queue &operator=(Queue &&other) noexcept {
		if (this != &other) {
			reset();
			moveFrom(other);
		}
		return *this;
	}

	[[nodiscard]] static Queue create(const QueueConfig &config) noexcept {
		Queue queue;
		if (!queue.start(config)) {
			queue.reset();
		}
		return queue;
	}

	void reset() noexcept {
		if (handle_ != nullptr) {
			vQueueDelete(handle_);
			handle_ = nullptr;
		}
		Strata::free(storage_);
		storage_ = nullptr;
		Strata::free(controlBlock_);
		controlBlock_ = nullptr;
		length_ = 0;
		storageBytes_ = 0;
	}

	[[nodiscard]] bool send(const T &value, TickType_t ticksToWait = 0) noexcept {
		return handle_ != nullptr && xQueueSend(handle_, &value, ticksToWait) == pdTRUE;
	}

	[[nodiscard]] bool receive(T &value, TickType_t ticksToWait = 0) noexcept {
		return handle_ != nullptr && xQueueReceive(handle_, &value, ticksToWait) == pdTRUE;
	}

	[[nodiscard]] bool sendFromISR(T const &value, BaseType_t *higherPriorityTaskWoken = nullptr) noexcept {
		return handle_ != nullptr && usage_ == QueueUsage::IsrAccessible &&
			xQueueSendFromISR(handle_, &value, higherPriorityTaskWoken) == pdTRUE;
	}

	[[nodiscard]] bool receiveFromISR(T &value, BaseType_t *higherPriorityTaskWoken = nullptr) noexcept {
		return handle_ != nullptr && usage_ == QueueUsage::IsrAccessible &&
			xQueueReceiveFromISR(handle_, &value, higherPriorityTaskWoken) == pdTRUE;
	}

	[[nodiscard]] Handle handle() const noexcept { return handle_; }
	[[nodiscard]] bool valid() const noexcept { return handle_ != nullptr; }
	[[nodiscard]] explicit operator bool() const noexcept { return valid(); }
	[[nodiscard]] std::size_t length() const noexcept { return length_; }
	[[nodiscard]] std::size_t storageBytes() const noexcept { return storageBytes_; }
	[[nodiscard]] Placement storagePlacement() const noexcept { return storagePlacement_; }
	[[nodiscard]] Region storageRegion() const noexcept { return Strata::regionOf(storage_); }
	[[nodiscard]] Placement controlPlacement() const noexcept { return Placement::Internal; }
	[[nodiscard]] Region controlRegion() const noexcept { return Strata::regionOf(controlBlock_); }
	[[nodiscard]] QueueUsage usage() const noexcept { return usage_; }

private:
	[[nodiscard]] bool start(const QueueConfig &config) noexcept {
		if (config.length == 0 ||
			config.length > static_cast<std::size_t>(std::numeric_limits<UBaseType_t>::max())) {
			return false;
		}
		if (sizeof(T) > static_cast<std::size_t>(std::numeric_limits<UBaseType_t>::max())) {
			return false;
		}
		if (config.length > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
			return false;
		}
		if (config.usage == QueueUsage::IsrAccessible &&
			config.storagePlacement != Placement::Internal) {
			return false;
		}

		const auto storageBytes = config.length * sizeof(T);
		storage_ = static_cast<std::uint8_t *>(Strata::allocate(AllocationRequest{
			.sizeBytes = storageBytes,
			.placement = config.storagePlacement,
			.alignment = alignof(T),
		}));
		if (storage_ == nullptr) {
			return false;
		}

		controlBlock_ = static_cast<StaticQueue_t *>(Strata::allocate(AllocationRequest{
			.sizeBytes = sizeof(StaticQueue_t),
			.placement = Placement::Internal,
			.alignment = alignof(StaticQueue_t),
		}));
		if (controlBlock_ == nullptr) {
			return false;
		}

		handle_ = xQueueCreateStatic(
			static_cast<UBaseType_t>(config.length),
			static_cast<UBaseType_t>(sizeof(T)),
			storage_,
			controlBlock_);
		if (handle_ == nullptr) {
			return false;
		}

		length_ = config.length;
		storageBytes_ = storageBytes;
		storagePlacement_ = config.storagePlacement;
		usage_ = config.usage;
		return true;
	}

	void moveFrom(Queue &other) noexcept {
		handle_ = std::exchange(other.handle_, nullptr);
		storage_ = std::exchange(other.storage_, nullptr);
		controlBlock_ = std::exchange(other.controlBlock_, nullptr);
		length_ = std::exchange(other.length_, 0);
		storageBytes_ = std::exchange(other.storageBytes_, 0);
		storagePlacement_ = other.storagePlacement_;
		usage_ = other.usage_;
	}

	Handle handle_{nullptr};
	std::uint8_t *storage_{nullptr};
	StaticQueue_t *controlBlock_{nullptr};
	std::size_t length_{0};
	std::size_t storageBytes_{0};
	Placement storagePlacement_{Placement::Internal};
	QueueUsage usage_{QueueUsage::TaskOnly};
};

} // namespace Strata::FreeRTOS
