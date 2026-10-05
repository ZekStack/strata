#pragma once

#include "FreeRTOS.h"

#include <cassert>
#include <cstdint>

using TaskHandle_t = void *;
using TaskFunction_t = void (*)(void *);

inline std::uint32_t fake_task_last_stack_depth = 0;
inline BaseType_t fake_task_last_affinity = -99;
inline UBaseType_t fake_task_high_water_mark = 64;
inline unsigned fake_task_create_calls = 0;
inline unsigned fake_task_delete_calls = 0;
inline unsigned fake_task_suspend_calls = 0;
inline unsigned fake_task_yield_calls = 0;
inline unsigned fake_task_running_yields = 0;
inline BaseType_t fake_task_running_core = -1;
inline bool fake_task_deleted_while_running = false;
inline TaskHandle_t fake_task_last_handle = nullptr;
inline bool fake_task_create_fail = false;

inline void fake_task_reset() {
    fake_task_last_stack_depth = 0;
    fake_task_last_affinity = -99;
    fake_task_high_water_mark = 64;
    fake_task_create_calls = 0;
    fake_task_delete_calls = 0;
    fake_task_suspend_calls = 0;
    fake_task_yield_calls = 0;
    fake_task_running_yields = 0;
    fake_task_running_core = -1;
    fake_task_deleted_while_running = false;
    fake_task_last_handle = nullptr;
    fake_task_create_fail = false;
}

inline void fake_task_yield() {
    ++fake_task_yield_calls;
    if (fake_task_running_yields > 0 && --fake_task_running_yields == 0) {
        fake_task_running_core = -1;
    }
}

#define taskYIELD() fake_task_yield()

inline TaskHandle_t xTaskCreateStaticPinnedToCore(
    TaskFunction_t,
    const char *,
    std::uint32_t stackDepth,
    void *,
    UBaseType_t,
    StackType_t *,
    StaticTask_t *controlBlock,
    BaseType_t affinity) {
    ++fake_task_create_calls;
    fake_task_last_stack_depth = stackDepth;
    fake_task_last_affinity = affinity;
    fake_task_last_handle = fake_task_create_fail ? nullptr : static_cast<TaskHandle_t>(controlBlock);
    return fake_task_last_handle;
}

inline TaskHandle_t xTaskCreateStatic(
    TaskFunction_t,
    const char *,
    std::uint32_t stackDepth,
    void *,
    UBaseType_t,
    StackType_t *,
    StaticTask_t *controlBlock) {
    ++fake_task_create_calls;
    fake_task_last_stack_depth = stackDepth;
    fake_task_last_handle = fake_task_create_fail ? nullptr : static_cast<TaskHandle_t>(controlBlock);
    return fake_task_last_handle;
}

inline void vTaskSuspend(TaskHandle_t handle) {
    assert(handle == fake_task_last_handle);
    ++fake_task_suspend_calls;
}

inline void vTaskDelete(TaskHandle_t handle) {
    assert(handle == fake_task_last_handle);
    fake_task_deleted_while_running = fake_task_running_core >= 0;
    ++fake_task_delete_calls;
}

inline UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t) {
    return fake_task_high_water_mark;
}
