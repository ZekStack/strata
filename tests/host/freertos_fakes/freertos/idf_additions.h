#pragma once

#include "task.h"

inline TaskHandle_t xTaskGetCurrentTaskHandleForCore(BaseType_t core) {
    assert(fake_task_suspend_calls > 0);
    return core == fake_task_running_core ? fake_task_last_handle : nullptr;
}
