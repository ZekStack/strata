# FreeRTOS binary semaphores

`Strata::FreeRTOS::BinarySemaphore` is an optional move-only owner for a FreeRTOS binary semaphore created with static FreeRTOS storage.

Include it explicitly:

```cpp
#include <strata/freertos/BinarySemaphore.h>
```

It is intentionally not included by `Strata.h`, so the core Strata API remains FreeRTOS-independent.

## Creation and initial state

```cpp
auto ready = Strata::FreeRTOS::BinarySemaphore::create();
if (!ready) {
    // Strata could not allocate the control block or FreeRTOS creation failed.
}
```

A newly created binary semaphore is **empty**. `tryTake()` therefore fails until the semaphore is given.

```cpp
assert(!ready.tryTake());
assert(ready.give());
assert(ready.tryTake());
```

Giving an already available binary semaphore returns `false` instead of hiding the FreeRTOS failure.

## Task-context API

```cpp
bool take(TickType_t ticksToWait = portMAX_DELAY) noexcept;
bool tryTake() noexcept;
bool give() noexcept;
```

`take()` forwards the requested FreeRTOS timeout. `tryTake()` is equivalent to `take(0)`.

The API uses `take`/`give` terminology rather than mutex-style `lock`/`unlock`: a binary semaphore is a signaling primitive and does not provide mutex ownership or priority-inheritance semantics.

## ISR API

```cpp
BaseType_t taskWoken = pdFALSE;
if (ready.giveFromISR(&taskWoken)) {
    // Signal published from the ISR.
}
```

The wrapper exposes:

```cpp
bool giveFromISR(BaseType_t *higherPriorityTaskWoken = nullptr) noexcept;
bool takeFromISR(BaseType_t *higherPriorityTaskWoken = nullptr) noexcept;
```

The optional `higherPriorityTaskWoken` pointer is forwarded directly to FreeRTOS. The caller remains responsible for performing the platform-appropriate ISR yield when FreeRTOS indicates that a higher-priority task was woken.

## Memory placement

The FreeRTOS `StaticSemaphore_t` control block is always allocated through Strata with:

```cpp
Strata::Placement::Internal
```

There is deliberately no placement configuration. Synchronization primitives may be used from ISR or cache-sensitive contexts, so Strata keeps the control storage internal rather than allowing PSRAM placement.

The requested and observed locations can be inspected with:

```cpp
ready.controlPlacement(); // Placement::Internal
ready.controlRegion();
```

## Ownership

`BinarySemaphore` is move-only. Its destructor and `reset()` delete the FreeRTOS semaphore before releasing the Strata-owned control block.

```cpp
auto first = Strata::FreeRTOS::BinarySemaphore::create();
auto second = std::move(first);

assert(!first);
assert(second);

second.reset();
```

Calling `reset()` repeatedly is safe.

## Requirements

The integration requires FreeRTOS static allocation support:

```text
configSUPPORT_STATIC_ALLOCATION == 1
```

The wrapper uses `xSemaphoreCreateBinaryStatic()` and never falls back to dynamic FreeRTOS semaphore allocation.
