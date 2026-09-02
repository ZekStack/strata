#pragma once

#include "../Allocation.h"
#include "../Diagnostics.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
}

#if configSUPPORT_STATIC_ALLOCATION != 1
#error "Strata FreeRTOS task integration requires configSUPPORT_STATIC_ALLOCATION == 1"
#endif

namespace Strata::FreeRTOS {

using TaskFunction = TaskFunction_t;
using TaskHandle = TaskHandle_t;

inline constexpr std::int32_t NoAffinity = -1;

struct TaskConfig {
    const char *name{"strata"};
    std::size_t stackBytes{0};
    Placement stackPlacement{Placement::Internal};
    UBaseType_t priority{tskIDLE_PRIORITY + 1};
    std::int32_t affinity{NoAffinity};
};

namespace Detail {

[[nodiscard]] constexpr bool roundStackBytes(
    std::size_t requestedBytes,
    std::size_t &roundedBytes,
    configSTACK_DEPTH_TYPE &depth) noexcept {
    if (requestedBytes == 0) {
        return false;
    }

    constexpr auto unit = sizeof(StackType_t);
    if (requestedBytes > std::numeric_limits<std::size_t>::max() - (unit - 1)) {
        return false;
    }

    roundedBytes = ((requestedBytes + unit - 1) / unit) * unit;
    const auto units = roundedBytes / unit;
    if (units > static_cast<std::size_t>(std::numeric_limits<configSTACK_DEPTH_TYPE>::max())) {
        return false;
    }

#if defined(ESP32)
    depth = static_cast<configSTACK_DEPTH_TYPE>(roundedBytes);
#else
    depth = static_cast<configSTACK_DEPTH_TYPE>(units);
#endif
    return true;
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
        (void)allocate(sizeBytes, placement);
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

        std::size_t roundedBytes = 0;
        configSTACK_DEPTH_TYPE depth = 0;
        if (!Detail::roundStackBytes(sizeBytes, roundedBytes, depth)) {
            return false;
        }

        auto *storage = Strata::allocate(AllocationRequest{
            .sizeBytes = roundedBytes,
            .placement = placement,
            .alignment = alignof(StackType_t),
        });
        if (storage == nullptr) {
            return false;
        }

        data_ = static_cast<StackType_t *>(storage);
        requestedBytes_ = sizeBytes;
        allocatedBytes_ = roundedBytes;
        depth_ = depth;
        placement_ = placement;
        return true;
    }

    void reset() noexcept {
        Strata::free(data_);
        data_ = nullptr;
        requestedBytes_ = 0;
        allocatedBytes_ = 0;
        depth_ = 0;
    }

    [[nodiscard]] StackType_t *data() noexcept { return data_; }
    [[nodiscard]] const StackType_t *data() const noexcept { return data_; }
    [[nodiscard]] std::size_t sizeBytes() const noexcept { return requestedBytes_; }
    [[nodiscard]] std::size_t allocatedBytes() const noexcept { return allocatedBytes_; }
    [[nodiscard]] configSTACK_DEPTH_TYPE depth() const noexcept { return depth_; }
    [[nodiscard]] Placement placement() const noexcept { return placement_; }
    [[nodiscard]] Region region() const noexcept { return Strata::regionOf(data_); }
    [[nodiscard]] explicit operator bool() const noexcept { return data_ != nullptr; }

private:
    void moveFrom(TaskStack &other) noexcept {
        data_ = std::exchange(other.data_, nullptr);
        requestedBytes_ = std::exchange(other.requestedBytes_, 0);
        allocatedBytes_ = std::exchange(other.allocatedBytes_, 0);
        depth_ = std::exchange(other.depth_, 0);
        placement_ = other.placement_;
    }

    StackType_t *data_{nullptr};
    std::size_t requestedBytes_{0};
    std::size_t allocatedBytes_{0};
    configSTACK_DEPTH_TYPE depth_{0};
    Placement placement_{Placement::Internal};
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

    [[nodiscard]] static Task create(
        TaskFunction function,
        void *context,
        const TaskConfig &config) noexcept {
        Task task;
        if (!task.start(function, context, config)) {
            task.reset();
        }
        return task;
    }

    void reset() noexcept {
        if (handle_ != nullptr) {
            vTaskDelete(handle_);
            handle_ = nullptr;
        }
        stack_.reset();
        Strata::free(controlBlock_);
        controlBlock_ = nullptr;
    }

    [[nodiscard]] TaskHandle handle() const noexcept { return handle_; }
    [[nodiscard]] bool valid() const noexcept { return handle_ != nullptr; }
    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    [[nodiscard]] std::size_t stackSizeBytes() const noexcept { return stack_.sizeBytes(); }
    [[nodiscard]] std::size_t allocatedStackBytes() const noexcept { return stack_.allocatedBytes(); }
    [[nodiscard]] Placement stackPlacement() const noexcept { return stack_.placement(); }
    [[nodiscard]] Region stackRegion() const noexcept { return stack_.region(); }

    [[nodiscard]] std::size_t stackHighWaterMarkBytes() const noexcept {
        if (handle_ == nullptr) {
            return 0;
        }
        return Detail::highWaterMarkBytes(uxTaskGetStackHighWaterMark(handle_));
    }

private:
    [[nodiscard]] bool start(TaskFunction function, void *context, const TaskConfig &config) noexcept {
        if (function == nullptr || config.name == nullptr || config.stackBytes == 0) {
            return false;
        }
        if (!stack_.allocate(config.stackBytes, config.stackPlacement)) {
            return false;
        }

        controlBlock_ = static_cast<StaticTask_t *>(Strata::allocate(AllocationRequest{
            .sizeBytes = sizeof(StaticTask_t),
            .placement = Placement::Internal,
            .alignment = alignof(StaticTask_t),
        }));
        if (controlBlock_ == nullptr) {
            stack_.reset();
            return false;
        }

#if defined(ESP32)
        handle_ = xTaskCreateStaticPinnedToCore(
            function,
            config.name,
            stack_.depth(),
            context,
            config.priority,
            stack_.data(),
            controlBlock_,
            config.affinity);
#else
        if (config.affinity != NoAffinity) {
            return false;
        }
        handle_ = xTaskCreateStatic(
            function,
            config.name,
            stack_.depth(),
            context,
            config.priority,
            stack_.data(),
            controlBlock_);
#endif

        return handle_ != nullptr;
    }

    void moveFrom(Task &other) noexcept {
        handle_ = std::exchange(other.handle_, nullptr);
        controlBlock_ = std::exchange(other.controlBlock_, nullptr);
        stack_ = std::move(other.stack_);
    }

    TaskHandle handle_{nullptr};
    StaticTask_t *controlBlock_{nullptr};
    TaskStack stack_{};
};

} // namespace Strata::FreeRTOS
