#pragma once

#include "../Allocation.h"
#include "../Diagnostics.h"

#include <limits>
#include <utility>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
}

#if configSUPPORT_STATIC_ALLOCATION != 1
#error "Strata FreeRTOS counting semaphore integration requires configSUPPORT_STATIC_ALLOCATION == 1"
#endif

namespace Strata::FreeRTOS {

class CountingSemaphore {
public:
	using Handle = SemaphoreHandle_t;

	CountingSemaphore() noexcept = default;

	~CountingSemaphore() noexcept {
		reset();
	}

	CountingSemaphore(const CountingSemaphore &) = delete;
	CountingSemaphore &operator=(const CountingSemaphore &) = delete;

	CountingSemaphore(CountingSemaphore &&other) noexcept {
		moveFrom(other);
	}

	CountingSemaphore &operator=(CountingSemaphore &&other) noexcept {
		if (this != &other) {
			reset();
			moveFrom(other);
		}
		return *this;
	}

	[[nodiscard]] static CountingSemaphore create(
	    std::size_t maxCount,
	    std::size_t initialCount = 0
	) noexcept {
		CountingSemaphore semaphore;
		if (!semaphore.start(maxCount, initialCount)) {
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
		maxCount_ = 0;
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
	[[nodiscard]] std::size_t maxCount() const noexcept { return maxCount_; }
	[[nodiscard]] Placement controlPlacement() const noexcept { return Placement::Internal; }
	[[nodiscard]] Region controlRegion() const noexcept { return Strata::regionOf(controlBlock_); }

private:
	[[nodiscard]] bool start(std::size_t maxCount, std::size_t initialCount) noexcept {
		if (maxCount == 0 || initialCount > maxCount ||
		    maxCount > static_cast<std::size_t>(std::numeric_limits<UBaseType_t>::max())) {
			return false;
		}

		controlBlock_ = static_cast<StaticSemaphore_t *>(Strata::allocate(AllocationRequest{
			.sizeBytes = sizeof(StaticSemaphore_t),
			.placement = Placement::Internal,
			.alignment = alignof(StaticSemaphore_t),
		}));
		if (controlBlock_ == nullptr) {
			return false;
		}

		handle_ = xSemaphoreCreateCountingStatic(
		    static_cast<UBaseType_t>(maxCount),
		    static_cast<UBaseType_t>(initialCount),
		    controlBlock_
		);
		if (handle_ != nullptr) {
			maxCount_ = maxCount;
		}
		return handle_ != nullptr;
	}

	void moveFrom(CountingSemaphore &other) noexcept {
		handle_ = std::exchange(other.handle_, nullptr);
		controlBlock_ = std::exchange(other.controlBlock_, nullptr);
		maxCount_ = std::exchange(other.maxCount_, 0);
	}

	Handle handle_{nullptr};
	StaticSemaphore_t *controlBlock_{nullptr};
	std::size_t maxCount_{0};
};

} // namespace Strata::FreeRTOS
