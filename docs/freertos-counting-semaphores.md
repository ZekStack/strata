# FreeRTOS counting semaphores

`Strata::FreeRTOS::CountingSemaphore` is a move-only owner for a FreeRTOS counting semaphore created with `xSemaphoreCreateCountingStatic()`.

## Creation

```cpp
#include <strata/freertos/CountingSemaphore.h>

auto slots = Strata::FreeRTOS::CountingSemaphore::create(20, 20);
if (!slots) {
    // Allocation or FreeRTOS creation failed.
}
```

The first argument is the maximum count and must be greater than zero. The initial count must not exceed it.

## Memory policy

The `StaticSemaphore_t` control block is always allocated through Strata with `Placement::Internal`. Synchronization metadata is intentionally not caller-placeable.

## Operations

`take()` and `give()` expose task-context operations. `tryTake()` is equivalent to `take(0)`. `takeFromISR()` and `giveFromISR()` expose the matching ISR-safe FreeRTOS APIs and forward the optional `higherPriorityTaskWoken` pointer.

Every operation returns `bool` so exhaustion, saturation, and invalid-owner states remain observable.

## Ownership

The wrapper owns both the FreeRTOS semaphore and its Strata-backed static control storage. Moving transfers ownership. `reset()` or destruction deletes the FreeRTOS semaphore and releases its control storage.
