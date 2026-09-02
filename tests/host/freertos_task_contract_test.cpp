#include <strata/freertos/Task.h>

#include <cassert>
#include <type_traits>

namespace {
void taskBody(void *) {}
}

int main() {
    using Strata::FreeRTOS::Task;
    using Strata::FreeRTOS::TaskConfig;
    using Strata::FreeRTOS::TaskStack;

    static_assert(!std::is_copy_constructible_v<TaskStack>);
    static_assert(std::is_move_constructible_v<TaskStack>);
    static_assert(!std::is_copy_constructible_v<Task>);
    static_assert(std::is_move_constructible_v<Task>);

    TaskStack stack{1001, Strata::Placement::Internal};
    assert(stack);
    assert(stack.sizeBytes() == 1001);
    assert(stack.allocatedBytes() == 1004);

    fake_task_reset();
    auto task = Task::create(taskBody, nullptr, TaskConfig{
        .name = "host",
        .stackBytes = 1001,
        .stackPlacement = Strata::Placement::Internal,
        .priority = 3,
        .affinity = -1,
    });
    assert(task);
    assert(fake_task_last_stack_depth == 251);
    assert(task.stackSizeBytes() == 1001);
    assert(task.stackHighWaterMarkBytes() == 64 * sizeof(StackType_t));

    Task moved = std::move(task);
    assert(!task);
    assert(moved);
    moved.reset();
    assert(fake_task_delete_calls == 1);
}
