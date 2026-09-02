# FreeRTOS mutexes

Strata provides optional move-only owners for statically created FreeRTOS mutexes:

```cpp
#include <strata/freertos/Mutex.h>

auto mutex = Strata::FreeRTOS::Mutex::create();
auto recursive = Strata::FreeRTOS::RecursiveMutex::create();
```

The integration requires:

- `configSUPPORT_STATIC_ALLOCATION == 1`;
- `configUSE_MUTEXES == 1`;
- `configUSE_RECURSIVE_MUTEXES == 1`.

The header checks those settings at compile time.

## Storage contract

The FreeRTOS `StaticSemaphore_t` control block is allocated through Strata with `Placement::Internal`. Mutexes intentionally do not expose a placement configuration because synchronization metadata is control state rather than movable bulk storage.

Creation uses `xSemaphoreCreateMutexStatic()` or `xSemaphoreCreateRecursiveMutexStatic()`. Strata does not use the dynamically allocating `xSemaphoreCreateMutex()` or `xSemaphoreCreateRecursiveMutex()` APIs.

`controlPlacement()` therefore always reports `Placement::Internal`, while `controlRegion()` reports the observed region when the backend can classify it.

## Ownership

`Mutex` and `RecursiveMutex` are move-only. `reset()` and destruction delete the FreeRTOS semaphore and release the Strata-owned static control block. Repeated `reset()` calls are safe.

```cpp
if (mutex.lock()) {
    // protected work
    mutex.unlock();
}

if (mutex.tryLock()) {
    mutex.unlock();
}
```

`RecursiveMutex` uses the recursive FreeRTOS take/give APIs and may be locked repeatedly by the owning task according to FreeRTOS semantics.

These wrappers are low-level ownership primitives. Higher-level libraries remain responsible for concurrency policy and lock ordering.
