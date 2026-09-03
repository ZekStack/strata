# API

The primary core include is:

```cpp
#include <Strata.h>
```

Optional integrations are intentionally separate:

```cpp
#include <strata/freertos/Task.h>
#include <strata/freertos/Queue.h>
#include <strata/freertos/Mutex.h>
#include <strata/freertos/BinarySemaphore.h>
#include <strata/arduinojson/Allocator.h>
#include <strata/pmr/MemoryResource.h>
```

## Placement and regions

- `Strata::Placement` — requested allocation policy.
- `Strata::Region` — observed memory region.
- `Strata::validPlacement(...)` — validate a placement value before persisting/decoding or using it in a request.
- `Strata::supports(...)` — placement, region, and capability support queries.
- `Strata::regionOf(ptr)` — inspect the region of a pointer when the platform can identify it.

Allocation/support APIs reject invalid `Placement` enum values instead of interpreting them as another policy.

See `placement.md` for the stable fallback, reallocation, capability-interaction, and safety semantics.

## Memory policy

`Strata::MemoryPolicy` is the shared consuming-library configuration vocabulary:

```cpp
Strata::MemoryPolicy policy{
    .allocation = Strata::Placement::PreferExternal,
    .taskStack = Strata::Placement::Internal,
};
```

Its defaults are `allocation = Placement::Default` and `taskStack = Placement::Internal`. `validMemoryPolicy()` validates both fields. The type has no global effect; libraries embed it in their own configuration objects. Safety requirements may tighten the policy for individual resources. See `memory-policy.md`.

## Raw allocation

```cpp
void *allocate(std::size_t sizeBytes, Placement placement = Placement::Default) noexcept;
void *allocate(const AllocationRequest &request) noexcept;
void *calloc(std::size_t count, std::size_t sizeBytes, Placement placement = Placement::Default) noexcept;
void *reallocate(void *ptr, std::size_t newSizeBytes, Placement placement = Placement::Default) noexcept;
void free(void *ptr) noexcept;
```

`AllocationRequest` adds alignment and required `Capability` flags.

## Diagnostics

`MemoryStats` exposes optional platform heap totals, free/minimum-free/largest-block values, plus derived current and peak used bytes per region.

Advanced allocation diagnostics are compile-time opt-in with `STRATA_ENABLE_ADVANCED_DIAGNOSTICS=1`:

```cpp
if (Strata::advancedDiagnosticsEnabled()) {
    auto total = Strata::allocationDiagnostics();
    auto preferred = Strata::allocationDiagnostics(Strata::Placement::PreferExternal);
    Strata::resetAllocationDiagnostics();
}
```

`AllocationDiagnostics` reports attempts, successes, failures, invalid requests, preferred-external fallbacks, and requested/successful/failed request bytes. Disabled builds retain the query API but return zeroed counters.

See `diagnostics.md` for precise counter semantics, concurrency, and the no-registry design boundary.

## Typed ownership

The typed helpers cover raw typed arrays, placement construction/destruction, unique ownership, and shared ownership while preserving Strata placement policy.

## Buffer

`Strata::Buffer` is a move-only owner for byte storage. Resize operations preserve the original requested placement policy.

## STL

`Strata::Allocator<T>` is a stateful standard allocator carrying placement intent. Convenience aliases and factories provide placement-aware vectors, strings, maps, and shared objects without hiding standard-library types.

## PMR

`Strata::MemoryResource` implements `std::pmr::memory_resource` and applies one Strata `Placement` policy to every allocation requested through that resource.

```cpp
Strata::MemoryResource resource{Strata::Placement::PreferExternal};
std::pmr::vector<std::pmr::string> values{&resource};
```

Nested PMR-aware containers propagate the same resource through standard polymorphic allocator semantics. Allocation size and alignment are forwarded to Strata unchanged, and deallocation is forwarded to `Strata::free()`.

PMR is opt-in through `<strata/pmr/MemoryResource.h>`. The header requires standard-library `<memory_resource>` support and exceptions because `std::pmr::memory_resource` reports allocation failure with `std::bad_alloc`.

See `pmr.md` for placement, lifetime, exception, and portability details.

## Capabilities

`Strata::Capability` is a bitmask of required allocation properties. Current portable capability names include DMA and executable memory. A request fails when the platform cannot satisfy the complete requirement set.

## FreeRTOS task memory

`Strata::FreeRTOS::TaskStack` owns explicitly placed stack storage.

`Strata::FreeRTOS::Task` owns the task handle, internal static control block, and stack storage. Task stack sizes and high-water marks are exposed in bytes. The task integration requires static allocation, task deletion, and stack high-water-mark support to be enabled in FreeRTOS.

A `Task` owner must be reset or destroyed from a different task context than the task it owns. Managed tasks must not self-delete because the owner must return from `vTaskDelete(handle)` before it can release the caller-owned static stack and control block.

This API is intentionally low-level. Worker remains the ZekStack orchestration layer for jobs, retries, cancellation, pools, and task lifecycle policy.

## FreeRTOS queues

`Strata::FreeRTOS::Queue<T>` is a move-only typed wrapper around `xQueueCreateStatic()`. Queue item storage follows `QueueConfig::storagePlacement` for task-only queues, while the `StaticQueue_t` control block remains internal.

`QueueUsage::IsrAccessible` requires internal item storage. `sendFromISR()` and `receiveFromISR()` reject task-only queues, and ISR-capable queues configured with external/default storage fail during creation. The underlying `QueueHandle_t` remains available through `handle()` for infrastructure users.

See `freertos-queues.md` for lifecycle and ISR safety details.

## FreeRTOS mutexes

`Strata::FreeRTOS::Mutex` and `Strata::FreeRTOS::RecursiveMutex` are move-only owners created through the FreeRTOS static mutex APIs. Their `StaticSemaphore_t` control storage is always allocated through Strata with `Placement::Internal`; there is intentionally no placement configuration for synchronization metadata.

Both wrappers expose `lock()`, `tryLock()`, `unlock()`, `reset()`, `handle()`, `controlPlacement()`, and `controlRegion()`. See `freertos-mutexes.md` for compile-time requirements and ownership semantics.

## FreeRTOS binary semaphores

`Strata::FreeRTOS::BinarySemaphore` is a move-only owner created through `xSemaphoreCreateBinaryStatic()`. Its `StaticSemaphore_t` control storage is always allocated through Strata with `Placement::Internal` and a newly created semaphore starts empty.

The task-context API exposes `take()`, `tryTake()`, and `give()`. `giveFromISR()` and `takeFromISR()` forward the optional `higherPriorityTaskWoken` pointer to FreeRTOS. Give/take operations return `bool` so an already-given semaphore or unavailable take is not silently hidden.

See `freertos-binary-semaphores.md` for signaling semantics, ISR usage, and ownership details.

## ArduinoJson

`Strata::ArduinoJson::Allocator` implements the ArduinoJson 7 custom allocator interface and forwards allocation, reallocation, and deallocation through a single Strata `Placement` policy.

```cpp
Strata::ArduinoJson::Allocator allocator{Strata::Placement::PreferExternal};
ArduinoJson::JsonDocument document{&allocator};
```

The allocator must outlive the `JsonDocument`. The integration is opt-in, is not included by `Strata.h`, and requires ArduinoJson major version 7 to be available to the translation unit.

See `arduinojson.md` for placement, failure, lifetime, and PSRAM details.

## Stable API boundary

The core API remains the stable base for ecosystem migrations. `v0.1.2` adds FreeRTOS binary semaphore ownership without weakening the placement, failure, or memory-policy contracts established by earlier releases.

The following semantic contracts are intentionally protected by tests and CI:

- placement and observed region remain distinct;
- required placement/capability constraints never silently weaken;
- `MemoryPolicy` is local configuration vocabulary, not a mutable global default;
- ordinary core APIs remain usable with exceptions disabled;
- allocation failure does not use abort/terminate as normal control flow;
- optional integrations are not pulled into `Strata.h`;
- platform-specific allocator flags do not become part of the core public vocabulary;
- advanced diagnostics remain optional and do not require an allocation registry;
- FreeRTOS mutex and binary-semaphore wrappers use static creation and internal Strata-backed control storage.

See `architecture.md` for the layering contract and `migration.md` for ecosystem adoption recipes.
