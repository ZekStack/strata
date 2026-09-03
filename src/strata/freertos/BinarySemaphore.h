#pragma once

#include "../Allocation.h"
#include "../Diagnostics.h"

#include <utility>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
}

#if configSUPPORT_STATIC_ALLOCATION != 1
#error "Strata FreeRTOS binary semaphore integration requires configSUPPORT_STATIC_ALLOCATION == 1"
#endif

namespace Strata::FreeRTOS {

class BinarySemaphore {
public:
	using Handle = SemaphoreHandle_t;

	BinarySemaphore() noexcept = default;

	~BinarySemaphore() noexcept {
		reset();
	}

	BinarySemaphore(const BinarySemaphore &) = delete;
	BinarySemaphore &operator=(const BinarySemaphore &) = delete;

	BinarySemaphore(BinarySemaphore &&other) noexcept {
		moveFrom(other);
	}

	BinarySemaphore &operator=(BinarySemaphore &&other) noexcept {
		if (this != &other) {
			reset();
			moveFrom(other);
		}
		return *this;
	}

	[[nodiscard]] static BinarySemaphore create() noexcept {
		BinarySemaphore semaphore;
		if (!semaphore.start()) {
			semaphore.reset();
		}
		return semaphore;
	}

	void reset() noexcept {
		if (handle_ != nullptr) {
			vSemaphoreDelete(handle_);
			handle_ = nullptr;
		}
		Strata::free(controlBlock_);
		controlBlock_ = nullptr;
	}

	[[nodiscard]] bool take(TickType_t ticksToWait = portMAX_DELAY) noexcept {
		return handle_ != nullptr && xSemaphoreTake(handle_, ticksToWait) == pdTRUE;
	}

	[[nodiscard]] bool tryTake() noexcept {
		return take(0);
	}

	[[nodiscard]] bool give() noexcept {
		return handle_ != nullptr && xSemaphoreGive(handle_) == pdTRUE;
	}

	[[nodiscard]] bool giveFromISR(BaseType_t *higherPriorityTaskWoken = nullptr) noexcept {
		return handle_ != nullptr && xSemaphoreGiveFromISR(handle_, higherPriorityTaskWoken) == pdTRUE;
	}

	[[nodiscard]] bool takeFromISR(BaseType_t *higherPriorityTaskWoken = nullptr) noexcept {
		return handle_ != nullptr && xSemaphoreTakeFromISR(handle_, higherPriorityTaskWoken) == pdTRUE;
	}

	[[nodiscard]] Handle handle() const noexcept { return handle_; }
	[[nodiscard]] bool valid() const noexcept { return handle_ != nullptr; }
	[[nodiscard]] explicit operator bool() const noexcept { return valid(); }
	[[nodiscard]] Placement controlPlacement() const noexcept { return Placement::Internal; }
	[[nodiscard]] Region controlRegion() const noexcept { return Strata::regionOf(controlBlock_); }

private:
	[[nodiscard]] bool start() noexcept {
		controlBlock_ = static_cast<StaticSemaphore_t *>(Strata::allocate(AllocationRequest{
			.sizeBytes = sizeof(StaticSemaphore_t),
			.placement = Placement::Internal,
			.alignment = alignof(StaticSemaphore_t),
		}));
		if (controlBlock_ == nullptr) {
			return false;
		}

		handle_ = xSemaphoreCreateBinaryStatic(controlBlock_);
		return handle_ != nullptr;
	}

	void moveFrom(BinarySemaphore &other) noexcept {
		handle_ = std::exchange(other.handle_, nullptr);
		controlBlock_ = std::exchange(other.controlBlock_, nullptr);
	}

	Handle handle_{nullptr};
	StaticSemaphore_t *controlBlock_{nullptr};
};

} // namespace Strata::FreeRTOS
