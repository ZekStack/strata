#include <strata/freertos/Task.h>
#include <esp_heap_caps.h>

#include <cassert>

namespace {
void taskBody(void *) {}
}

int main() {
    using Strata::FreeRTOS::Task;
    using Strata::FreeRTOS::TaskConfig;
    using Strata::Placement;
    using Strata::Region;

    fake_heap_caps_reset();
    fake_task_reset();

    auto internal = Task::create(taskBody, nullptr, TaskConfig{
        .name = "internal",
        .stackBytes = 2048,
        .stackPlacement = Placement::Internal,
        .priority = 2,
        .affinity = 0,
    });
    assert(internal);
    assert(internal.stackRegion() == Region::Internal);
    assert(fake_task_last_stack_depth == 2048);
    assert(fake_task_last_affinity == 0);
    assert(internal.stackHighWaterMarkBytes() == 64);
    internal.reset();

    fake_heap_caps_reset();
    fake_task_reset();
    auto preferredExternal = Task::create(taskBody, nullptr, TaskConfig{
        .name = "external",
        .stackBytes = 4096,
        .stackPlacement = Placement::PreferExternal,
    });
    assert(preferredExternal);
    assert(preferredExternal.stackPlacement() == Placement::PreferExternal);
    assert(preferredExternal.stackRegion() == Region::External);
    preferredExternal.reset();

    fake_heap_caps_reset();
    fake_task_reset();
    fake_heap_caps_fail_external = true;
    auto fallback = Task::create(taskBody, nullptr, TaskConfig{
        .name = "fallback",
        .stackBytes = 4096,
        .stackPlacement = Placement::PreferExternal,
    });
    assert(fallback);
    assert(fallback.stackRegion() == Region::Internal);
    fallback.reset();

    fake_heap_caps_reset();
    fake_task_reset();
    fake_heap_caps_fail_external = true;
    auto strict = Task::create(taskBody, nullptr, TaskConfig{
        .name = "strict",
        .stackBytes = 4096,
        .stackPlacement = Placement::RequireExternal,
    });
    assert(!strict);
    assert(fake_task_create_calls == 0);

    fake_heap_caps_reset();
    fake_task_reset();
    fake_heap_caps_external_total = 0;
    auto unavailable = Task::create(taskBody, nullptr, TaskConfig{
        .name = "missing",
        .stackBytes = 4096,
        .stackPlacement = Placement::RequireExternal,
    });
    assert(!unavailable);
    assert(fake_task_create_calls == 0);
}
