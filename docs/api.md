# API

The primary core include is:

```cpp
#include <Strata.h>
```

Optional FreeRTOS integration is intentionally separate:

```cpp
#include <strata/freertos/Task.h>
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

See the specialized documents for exact semantics and safety boundaries.
