#pragma once

#include "../Allocation.h"
#include "../Diagnostics.h"

#include <utility>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
}

#if configSUPPORT_STATIC_ALLOCATION != 1
#error "Strata FreeRTOS mutex integration requires configSUPPORT_STATIC_ALLOCATION == 1"
#endif

#if configUSE_MUTEXES != 1
#error "Strata FreeRTOS mutex integration requires configUSE_MUTEXES == 1"
#endif

#if configUSE_RECURSIVE_MUTEXES != 1
#error "Strata FreeRTOS recursive mutex integration requires configUSE_RECURSIVE_MUTEXES == 1"
#endif

namespace Strata::FreeRTOS {

class Mutex {
public:
	using Handle = SemaphoreHandle_t;

	Mutex() noexcept = default;

	~Mutex() noexcept {
		reset();
	}

	Mutex(const Mutex &) = delete;
	Mutex &operator=(const Mutex &) = delete;

	Mutex(Mutex &&other) noexcept {
		moveFrom(other);
	}

	Mutex &operator=(Mutex &&other) noexcept {
		if (this != &other) {
			reset();
			moveFrom(other);
		}
		return *this;
	}

	[[nodiscard]] static Mutex create() noexcept {
		Mutex mutex;
		if (!mutex.start()) {
			mutex.reset();
		}
		return mutex;
	}

	void reset() noexcept {
		if (handle_ != nullptr) {
			vSemaphoreDelete(handle_);
			handle_ = nullptr;
		}
		Strata::free(controlBlock_);
		controlBlock_ = nullptr;
	}

	[[nodiscard]] bool lock(TickType_t ticksToWait = portMAX_DELAY) noexcept {
		return handle_ != nullptr && xSemaphoreTake(handle_, ticksToWait) == pdTRUE;
	}

	[[nodiscard]] bool tryLock() noexcept {
		return lock(0);
	}

	void unlock() noexcept {
		if (handle_ != nullptr) {
			(void)xSemaphoreGive(handle_);
		}
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

		handle_ = xSemaphoreCreateMutexStatic(controlBlock_);
		return handle_ != nullptr;
	}

	void moveFrom(Mutex &other) noexcept {
		handle_ = std::exchange(other.handle_, nullptr);
		controlBlock_ = std::exchange(other.controlBlock_, nullptr);
	}

	Handle handle_{nullptr};
	StaticSemaphore_t *controlBlock_{nullptr};
};

class RecursiveMutex {
public:
	using Handle = SemaphoreHandle_t;

	RecursiveMutex() noexcept = default;

	~RecursiveMutex() noexcept {
		reset();
	}

	RecursiveMutex(const RecursiveMutex &) = delete;
	RecursiveMutex &operator=(const RecursiveMutex &) = delete;

	RecursiveMutex(RecursiveMutex &&other) noexcept {
		moveFrom(other);
	}

	RecursiveMutex &operator=(RecursiveMutex &&other) noexcept {
		if (this != &other) {
			reset();
			moveFrom(other);
		}
		return *this;
	}

	[[nodiscard]] static RecursiveMutex create() noexcept {
		RecursiveMutex mutex;
		if (!mutex.start()) {
			mutex.reset();
		}
		return mutex;
	}

	void reset() noexcept {
		if (handle_ != nullptr) {
			vSemaphoreDelete(handle_);
			handle_ = nullptr;
		}
		Strata::free(controlBlock_);
		controlBlock_ = nullptr;
	}

	[[nodiscard]] bool lock(TickType_t ticksToWait = portMAX_DELAY) noexcept {
		return handle_ != nullptr && xSemaphoreTakeRecursive(handle_, ticksToWait) == pdTRUE;
	}

	[[nodiscard]] bool tryLock() noexcept {
		return lock(0);
	}

	void unlock() noexcept {
		if (handle_ != nullptr) {
			(void)xSemaphoreGiveRecursive(handle_);
		}
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

		handle_ = xSemaphoreCreateRecursiveMutexStatic(controlBlock_);
		return handle_ != nullptr;
	}

	void moveFrom(RecursiveMutex &other) noexcept {
		handle_ = std::exchange(other.handle_, nullptr);
		controlBlock_ = std::exchange(other.controlBlock_, nullptr);
	}

	Handle handle_{nullptr};
	StaticSemaphore_t *controlBlock_{nullptr};
};

} // namespace Strata::FreeRTOS
