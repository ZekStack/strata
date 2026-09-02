# FreeRTOS queues

Strata's queue integration is optional and does not change the core `<Strata.h>` dependency surface.

```cpp
#include <strata/freertos/Queue.h>
```

`Strata::FreeRTOS::Queue<T>` is a move-only owner around a statically created FreeRTOS queue. `T` must be trivially copyable because FreeRTOS queues transfer items by copying their bytes.

## Placement model

Queue item storage and the FreeRTOS queue control structure have different placement rules:

- item storage follows `QueueConfig::storagePlacement` for task-only queues;
- the `StaticQueue_t` control structure is always allocated in internal memory;
- `PreferExternal` may place task-only item storage in PSRAM and fall back to internal RAM;
- `RequireExternal` fails if external item storage cannot be allocated.

The requested storage placement and observed storage/control regions are exposed separately through `storagePlacement()`, `storageRegion()`, `controlPlacement()`, and `controlRegion()`.

## ISR safety

`QueueUsage::IsrAccessible` is intentionally stricter than `TaskOnly`. ISR-accessible queues currently require `Placement::Internal` item storage, and their control block is internal as well. Strata rejects an ISR-accessible queue configured with `Default`, `PreferExternal`, or `RequireExternal` storage because external/cache-backed queue memory has not been proven safe for ISR execution across supported targets.

The wrapper's `sendFromISR()` and `receiveFromISR()` methods also refuse operations on queues created as `TaskOnly`.

Infrastructure users may access the underlying `QueueHandle_t` through `handle()`. Doing so bypasses the wrapper's ISR-mode checks; callers must not use an externally backed task-only queue from an ISR.

## Lifetime

`Queue<T>` owns the queue handle, item-storage allocation, and internal static control block. `reset()` deletes the FreeRTOS queue before releasing both Strata allocations. Moving a queue transfers ownership without relocating either backing allocation.

This API is intentionally low-level. Higher-level scheduling, retries, cancellation, worker pools, and orchestration remain outside Strata.
