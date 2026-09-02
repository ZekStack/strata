#pragma once

#include "../Allocation.h"
#include "../Diagnostics.h"

#if __has_include(<freertos/FreeRTOS.h>) && __has_include(<freertos/task.h>)
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#else
#error "Strata FreeRTOS integration requires FreeRTOS headers"
#endif

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

#if !defined(configSUPPORT_STATIC_ALLOCATION) || (configSUPPORT_STATIC_ALLOCATION != 1)
#error "Strata task integration requires configSUPPORT_STATIC_ALLOCATION=1"
#endif

namespace Strata::FreeRTOS {

namespace Detail {

[[nodiscard]] constexpr std::size_t roundStackBytes(std::size_t bytes) noexcept {
    if (bytes == 0) {
        return 0;
    }
    constexpr std::size_t unit = sizeof(StackType_t);
    const auto remainder = bytes % unit;
    if (remainder == 0) {
        return bytes;
    }
    const auto padding = unit - remainder;
    return bytes > (std::numeric_limits<std::size_t>::max() - padding) ? 0 : bytes + padding;
}

[[nodiscard]] constexpr std::uint32_t stackDepthArgument(std::size_t allocatedBytes) noexcept {
#if defined(ESP32)
    return static_cast<std::uint32_t>(allocatedBytes);
#else
    return static_cast<std::uint32_t>(allocatedBytes / sizeof(StackType_t));
#endif
}

[[nodiscard]] constexpr std::size_t highWaterMarkBytes(UBaseType_t value) noexcept {
#if defined(ESP32)
    return static_cast<std::size_t>(value);
#else
    return static_cast<std::size_t>(value) * sizeof(StackType_t);
#endif
}

} // namespace Detail

class TaskStack {
public:
    TaskStack() noexcept = default;

    explicit TaskStack(std::size_t sizeBytes, Placement placement = Placement::Internal) noexcept {
        allocate(sizeBytes, placement);
    }

    ~TaskStack() noexcept {
        reset();
    }

    TaskStack(const TaskStack &) = delete;
    TaskStack &operator=(const TaskStack &) = delete;

    TaskStack(TaskStack &&other) noexcept {
        moveFrom(other);
    }

    TaskStack &operator=(TaskStack &&other) noexcept {
        if (this != &other) {
            reset();
            moveFrom(other);
        }
        return *this;
    }

    [[nodiscard]] bool allocate(std::size_t sizeBytes, Placement placement = Placement::Internal) noexcept {
        reset();
        placement_ = placement;
        requestedBytes_ = sizeBytes;
        allocatedBytes_ = Detail::roundStackBytes(sizeBytes);
        if (allocatedBytes_ == 0 || allocatedBytes_ > std::numeric_limits<std::uint32_t>::max()) {
            allocatedBytes_ = 0;
            return false;
        }

        data_ = static_cast<StackType_t *>(Strata::allocate(AllocationRequest{
            .sizeBytes = allocatedBytes_,
            .placement = placement_,
            .alignment = alignof(StackType_t),
            .capabilities = Capability::None,
        }));
        if (data_ == nullptr) {
            allocatedBytes_ = 0;
            return false;
        }
        return true;
    }

    void reset() noexcept {
        Strata::free(data_);
        data_ = nullptr;
        requestedBytes_ = 0;
        allocatedBytes_ = 0;
    }

    [[nodiscard]] explicit operator bool() const noexcept { return data_ != nullptr; }
    [[nodiscard]] StackType_t *data() noexcept { return data_; }
    [[nodiscard]] const StackType_t *data() const noexcept { return data_; }
    [[nodiscard]] std::size_t sizeBytes() const noexcept { return requestedBytes_; }
    [[nodiscard]] std::size_t allocatedBytes() const noexcept { return allocatedBytes_; }
    [[nodiscard]] Placement placement() const noexcept { return placement_; }
    [[nodiscard]] Region region() const noexcept { return Strata::regionOf(data_); }

private:
    void moveFrom(TaskStack &other) noexcept {
        data_ = std::exchange(other.data_, nullptr);
        requestedBytes_ = std::exchange(other.requestedBytes_, 0);
        allocatedBytes_ = std::exchange(other.allocatedBytes_, 0);
        placement_ = other.placement_;
    }

    StackType_t *data_{nullptr};
    std::size_t requestedBytes_{0};
    std::size_t allocatedBytes_{0};
    Placement placement_{Placement::Internal};
};

struct TaskConfig {
    const char *name{"strata"};
    std::size_t stackBytes{4096};
    Placement stackPlacement{Placement::Internal};
    UBaseType_t priority{1};
    BaseType_t affinity{-1};
};

class Task {
public:
    Task() noexcept = default;

    ~Task() noexcept {
        reset();
    }

    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    Task(Task &&other) noexcept {
        moveFrom(other);
    }

    Task &operator=(Task &&other) noexcept {
        if (this != &other) {
            reset();
            moveFrom(other);
        }
        return *this;
    }

    [[nodiscard]] static Task create(TaskFunction_t function, void *context, const TaskConfig &config = {}) noexcept {
        Task task;
        if (function == nullptr || config.name == nullptr || config.stackBytes == 0) {
            return task;
        }
        if (!task.stack_.allocate(config.stackBytes, config.stackPlacement)) {
            return task;
        }

        task.controlBlock_ = static_cast<StaticTask_t *>(Strata::allocate(AllocationRequest{
            .sizeBytes = sizeof(StaticTask_t),
            .placement = Placement::Internal,
            .alignment = alignof(StaticTask_t),
            .capabilities = Capability::None,
        }));
        if (task.controlBlock_ == nullptr) {
            task.stack_.reset();
            return task;
        }

#if defined(ESP32)
        task.handle_ = xTaskCreateStaticPinnedToCore(
            function,
            config.name,
            Detail::stackDepthArgument(task.stack_.allocatedBytes()),
            context,
            config.priority,
            task.stack_.data(),
            task.controlBlock_,
            config.affinity);
#else
        task.handle_ = xTaskCreateStatic(
            function,
            config.name,
            Detail::stackDepthArgument(task.stack_.allocatedBytes()),
            context,
            config.priority,
            task.stack_.data(),
            task.controlBlock_);
#endif
        if (task.handle_ == nullptr) {
            Strata::free(task.controlBlock_);
            task.controlBlock_ = nullptr;
            task.stack_.reset();
        }
        return task;
    }

    void reset() noexcept {
        if (handle_ != nullptr) {
            vTaskDelete(handle_);
            handle_ = nullptr;
        }
        Strata::free(controlBlock_);
        controlBlock_ = nullptr;
        stack_.reset();
    }

    [[nodiscard]] explicit operator bool() const noexcept { return handle_ != nullptr; }
    [[nodiscard]] TaskHandle_t handle() const noexcept { return handle_; }
    [[nodiscard]] std::size_t stackSizeBytes() const noexcept { return stack_.sizeBytes(); }
    [[nodiscard]] Placement stackPlacement() const noexcept { return stack_.placement(); }
    [[nodiscard]] Region stackRegion() const noexcept { return stack_.region(); }

    [[nodiscard]] std::size_t stackHighWaterMarkBytes() const noexcept {
        if (handle_ == nullptr) {
            return 0;
        }
        return Detail::highWaterMarkBytes(uxTaskGetStackHighWaterMark(handle_));
    }

private:
    void moveFrom(Task &other) noexcept {
        handle_ = std::exchange(other.handle_, nullptr);
        controlBlock_ = std::exchange(other.controlBlock_, nullptr);
        stack_ = std::move(other.stack_);
    }

    TaskHandle_t handle_{nullptr};
    StaticTask_t *controlBlock_{nullptr};
    TaskStack stack_{};
};

} // namespace Strata::FreeRTOS
