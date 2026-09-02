# Strata architecture

## Purpose

Strata is the standalone low-level memory placement layer for the ZekStack ecosystem. It lets applications and reusable libraries describe allocation intent in portable C++ terms while platform backends translate that intent to the native allocator.

Strata does not own application scheduling, business logic, logging policy, networking, persistence, or device behavior.

The dependency direction is intentionally one-way:

```text
application / firmware
        ↓
Worker / Signal / Trace / Tempo / other libraries
        ↓
Strata
        ↓
standard allocator or platform allocator
```

Optional adapters add narrow integration boundaries without becoming core dependencies:

```text
FreeRTOS    → strata/freertos/*
ArduinoJson → strata/arduinojson/*
std::pmr    → strata/pmr/*
```

None of those optional headers are included by `Strata.h`.

## Stable public vocabulary

### Placement

`Placement` describes requested policy: `Default`, `Internal`, `PreferExternal`, or `RequireExternal`.

### Region

`Region` describes observed storage location: `Unknown`, `Internal`, or `External`.

Requested placement and actual region are deliberately different concepts. A `PreferExternal` allocation may legally resolve to `Region::Internal`; `RequireExternal` may not.

### Capability

`Capability` describes required allocation properties such as DMA or executable memory. Capabilities are hard requirements and are preserved across placement fallback.

The public core vocabulary intentionally contains no ESP-IDF heap flags, PSRAM-specific enum values, FreeRTOS types, ArduinoJson types, or PMR types.

## Layering

### Core allocation

`AllocationRequest`, `allocate`, `calloc`, `reallocate`, and `free` form the lowest public layer. Ordinary core allocation APIs are `noexcept` and report allocation failure with `nullptr`. Zero-size raw allocation has deterministic Strata semantics rather than relying on implementation-defined allocator sentinels.

### Diagnostics

`regionOf`, `supports`, and `memoryStats` expose placement support and observed heap state without exposing native allocator APIs. `memoryStats` also derives current/peak region usage where the platform exposes total, free, and minimum-free values.

Advanced allocation counters are compile-time opt-in. When enabled they use fixed atomic counters for request/success/failure/fallback traffic; when disabled the allocation path does not update diagnostic state. No pointer registry, dynamic diagnostic allocation, or Strata-owned lock is required.

### Typed ownership

Typed allocation/construction helpers and `UniquePtr` build object lifetime on top of the same raw allocation contract.

### Buffer

`Buffer` is move-only owned byte storage. Its requested placement remains part of the object policy and is preserved across resize attempts.

### Standard-library allocator

`Allocator<T>` carries placement into standard containers. It follows the standard allocator failure contract where exceptions are enabled. Exception-disabled standard containers are not treated as a deterministic OOM recovery mechanism.

### PMR

`MemoryResource` is an optional `std::pmr::memory_resource` adapter. It is excluded from `Strata.h` because PMR availability and exception requirements are separate from the core embedded contract.

### ArduinoJson

The optional ArduinoJson 7 allocator adapter forwards document allocation through a Strata placement policy. ArduinoJson remains a caller-supplied dependency and the allocator lifetime remains explicit.

### FreeRTOS tasks and queues

The optional FreeRTOS layer owns low-level static task/queue memory and exposes native handles for infrastructure integration. It does not replace Worker or become a general task framework.

Task stacks may use external memory only when the calling subsystem can tolerate the platform's cache/flash restrictions. ISR-accessible queues require internal item storage under the current contract.

## Platform boundary

### Generic backend

The generic backend provides the portable contract for host builds:

- `Default`, `Internal`, and `PreferExternal` use the process heap;
- `RequireExternal` is unsupported and fails;
- embedded-only capabilities are reported unsupported;
- region classification remains conservative.

This backend is primarily a portability baseline and contract-test target. It does not pretend that host memory has ESP32 PSRAM semantics.

### ESP32 backend

The ESP32 backend translates Strata requests to ESP-IDF heap capabilities:

- internal placement requires internal byte-addressable memory;
- external placement requires SPIRAM;
- required capabilities are combined with placement;
- `PreferExternal` explicitly tries external first and then internal;
- `RequireExternal` performs no internal fallback.

ESP-IDF headers and `MALLOC_CAP_*` details stay inside the backend implementation. Reusable callers should not need to depend on those symbols.

## Failure contracts

The stable failure rules are:

1. Raw allocation, typed ownership, and `Buffer` do not require exceptions for ordinary allocation failure.
2. Required placement and capability constraints never silently degrade.
3. Failed `reallocate` leaves the original allocation valid.
4. `Allocator<T>` follows standard allocator behavior; exception-enabled OOM throws `std::bad_alloc`.
5. PMR follows `std::pmr::memory_resource` semantics and therefore requires exceptions.
6. Strata production code must not abort or terminate as an ordinary allocation-failure policy.
7. Optional integration failure must remain local to the integration rather than changing core behavior.

## Safety rules

1. External memory is never assumed safe for ISR, DMA, flash/cache-disabled, or peripheral-sensitive use merely because it is allocatable.
2. Callers must choose `Internal` for contexts that require internal accessibility.
3. `PreferExternal` is appropriate only when internal fallback is semantically acceptable.
4. `RequireExternal` is appropriate only when failure is preferable to consuming internal memory.
5. Diagnostics distinguish requested policy from observed region.
6. Strata has no required global initialization and no hidden mutable global default placement.
7. Advanced diagnostics are opt-in and must not introduce allocation registries, dynamic diagnostic allocation, allocator recursion, or mandatory allocation-path overhead.

## API stability boundary

Phase 12 established the stable core API for ecosystem migration. The completed `v0.1.0` roadmap adds advanced diagnostics as an additive layer while preserving these names and semantics:

- `Placement`, `Region`, and `Capability`;
- `AllocationRequest` and the raw allocation functions;
- diagnostics and support queries;
- typed ownership, `Buffer`, `Allocator<T>`, and STL factories;
- optional `MemoryResource`, ArduinoJson, FreeRTOS task, and FreeRTOS queue adapters behind their explicit include paths.

Future work should not make platform-specific types part of the core vocabulary or weaken existing placement/failure guarantees.
