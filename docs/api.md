# API

The primary core include is:

```cpp
#include <Strata.h>
```

Optional integrations are intentionally separate:

```cpp
#include <strata/freertos/Task.h>
#include <strata/freertos/Queue.h>
#include <strata/arduinojson/Allocator.h>
```

## Placement and regions

- `Strata::Placement` — requested allocation policy.
- `Strata::Region` — observed memory region.
- `Strata::supports(...)` — placement, region, and capability support queries.
- `Strata::regionOf(ptr)` — inspect the region of a pointer when the platform can identify it.

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

`MemoryStats` exposes available total, free, minimum-free, and largest-free-block values where the platform can provide them.

## Typed ownership

The typed helpers cover raw typed arrays, placement construction/destruction, unique ownership, and shared ownership while preserving Strata placement policy.

## Buffer

`Strata::Buffer` is a move-only owner for byte storage. Resize operations preserve the original requested placement policy.

## STL

`Strata::Allocator<T>` is a stateful standard allocator carrying placement intent. Convenience aliases and factories provide placement-aware vectors, strings, maps, and shared objects without hiding standard-library types.

## Capabilities

`Strata::Capability` is a bitmask of required allocation properties. Current portable capability names include DMA and executable memory. A request fails when the platform cannot satisfy the complete requirement set.

## FreeRTOS task memory

`Strata::FreeRTOS::TaskStack` owns explicitly placed stack storage.

`Strata::FreeRTOS::Task` owns the task handle, internal static control block, and stack storage. Task stack sizes and high-water marks are exposed in bytes.

This API is intentionally low-level. Worker remains the ZekStack orchestration layer for jobs, retries, cancellation, pools, and task lifecycle policy.

## FreeRTOS queues

`Strata::FreeRTOS::Queue<T>` is a move-only typed wrapper around `xQueueCreateStatic()`. Queue item storage follows `QueueConfig::storagePlacement` for task-only queues, while the `StaticQueue_t` control block remains internal.

`QueueUsage::IsrAccessible` requires internal item storage. `sendFromISR()` and `receiveFromISR()` reject task-only queues, and ISR-capable queues configured with external/default storage fail during creation. The underlying `QueueHandle_t` remains available through `handle()` for infrastructure users.

See `freertos-queues.md` for lifecycle and ISR safety details.

## ArduinoJson

`Strata::ArduinoJson::Allocator` implements the ArduinoJson 7 custom allocator interface and forwards allocation, reallocation, and deallocation through a single Strata `Placement` policy.

```cpp
Strata::ArduinoJson::Allocator allocator{Strata::Placement::PreferExternal};
ArduinoJson::JsonDocument document{&allocator};
```

The allocator must outlive the `JsonDocument`. The integration is opt-in, is not included by `Strata.h`, and requires ArduinoJson major version 7 to be available to the translation unit.

See `arduinojson.md` for placement, failure, lifetime, and PSRAM details.

See the specialized documents for exact semantics and safety boundaries.
