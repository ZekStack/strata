# FreeRTOS task memory integration

Phase 8 adds an optional, low-level FreeRTOS task-memory layer. Nothing in Strata's core headers depends on FreeRTOS; users opt in by including `strata/freertos/Task.h`.

## Task stacks

`Strata::FreeRTOS::TaskStack` is a move-only owner of raw FreeRTOS stack storage. Its public sizes are always bytes. The allocation records the requested `Placement`, while `region()` reports the current physical region through Strata diagnostics.

Stack storage is allocated and released through the normal Strata allocation pair, so ESP32 internal/external heap-capability allocation is matched with `heap_caps_free()` through the backend.

The requested byte count is rounded up to a whole `StackType_t` for storage. `sizeBytes()` reports the requested byte count and `allocatedBytes()` reports the rounded allocation.

## Minimal task creation

`TaskConfig` contains only:

- task name;
- stack bytes;
- stack placement;
- priority;
- affinity.

`Task::create()` uses FreeRTOS static task creation so Strata can supply the task stack explicitly. The `StaticTask_t` control block is deliberately allocated in internal memory. On ESP32, `xTaskCreateStaticPinnedToCore()` receives the stack size in ESP-IDF's byte units. On other FreeRTOS ports, Strata converts the public byte count to `StackType_t` units before calling `xTaskCreateStatic()`.

`Task::stackHighWaterMarkBytes()` similarly normalizes the FreeRTOS high-water mark to bytes.

## Lifetime boundary

`Task` is move-only and owns the task handle, static control block, and stack memory. `reset()`/destruction calls `vTaskDelete(handle)` before releasing those buffers. A task managed by this wrapper must therefore not independently self-delete with `vTaskDelete(nullptr)`; ownership must remain with the `Task` object.

This is intentionally a primitive, not an orchestration framework. Jobs, callbacks, retry, cancellation, worker pools, cleanup services, and scheduling policy belong in higher-level libraries. ZekStack users should continue to use **Worker** for normal asynchronous job orchestration; Worker can later use this Strata layer for its low-level stack placement.

## External-memory safety

External stack placement is appropriate only for tasks whose execution and platform configuration support external stacks. It does not make code cache-independent. Tasks that can run during flash/cache-disabled windows, or otherwise require internal-only execution/storage, should use `Placement::Internal`.

`PreferExternal` may fall back to internal memory. `RequireExternal` fails task creation when the external stack cannot be allocated; the requirement is never silently weakened.
