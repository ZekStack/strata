# Strata architecture

## Purpose

Strata is a standalone memory placement and allocation utility library. Its responsibility is to let callers express memory requirements in portable terms while platform backends translate those requirements into native allocation and memory APIs.

Strata must not depend on Worker, Signal, Trace, Tempo, Passage, or any other ZekStack library. The intended dependency direction is:

```text
application
    ↓
Worker / Signal / Trace / Tempo / other libraries
    ↓
Strata
    ↓
standard allocator / platform allocator / optional FreeRTOS integration
```

## Public vocabulary

### Placement

`Placement` describes caller intent.

- `Default` — use normal platform behavior.
- `Internal` — the operation must use internal/default-local memory.
- `PreferExternal` — try external memory first and fall back to internal memory when necessary.
- `RequireExternal` — external memory is mandatory and failure must be surfaced when unavailable.

### Region

`Region` describes an observed allocation location.

- `Unknown`
- `Internal`
- `External`

A requested placement must never be confused with the actual region. For example, `PreferExternal` may legally produce `Region::Internal` after fallback, while `RequireExternal` may not.

## Raw allocation contract

Phase 2 provides the non-owning raw allocation layer used by later RAII, allocator, buffer, and integration APIs.

### AllocationRequest

`AllocationRequest` carries three constraints:

- `sizeBytes` — requested byte count;
- `placement` — requested placement policy;
- `alignment` — requested power-of-two alignment, defaulting to `alignof(std::max_align_t)`.

The request is deliberately extensible so later capability constraints can be added without replacing the placement vocabulary.

### Failure and zero-size behavior

The ordinary raw allocation API is `noexcept` and reports failure with `nullptr`.

- `allocate(0, ...)` returns `nullptr`.
- `calloc(0, size, ...)` and `calloc(count, 0, ...)` return `nullptr`.
- `calloc` rejects `count * size` overflow before entering a platform backend.
- alignment must be a non-zero power of two; invalid or platform-unsupported alignment returns `nullptr`.
- `free(nullptr)` is a no-op.

This avoids platform-dependent zero-size sentinel allocations and gives embedded callers one deterministic contract.

### Reallocation and migration

`reallocate(ptr, newSize, placement)` may move an allocation in order to satisfy the new placement request.

- `reallocate(nullptr, size, placement)` is equivalent to a new default-aligned allocation.
- `reallocate(ptr, 0, placement)` frees `ptr` and returns `nullptr`.
- `Internal` may migrate an ESP32 allocation into internal memory.
- `PreferExternal` first attempts migration/resizing in external memory and retries internally if the external request fails.
- `RequireExternal` attempts only external memory and never degrades.
- on failure, the original allocation remains valid and owned by the caller.

Phase 2 `reallocate` is defined for default-aligned allocations produced by `allocate(size, placement)`, `calloc`, or `reallocate`. Over-aligned allocations produced through `AllocationRequest` must be released with `Strata::free` and are not accepted by `reallocate` yet. This avoids pretending that portable `realloc` APIs preserve arbitrary alignment metadata.

## Platform boundary

### Generic backend

The generic backend is the portable baseline.

- `Default` and `Internal` use standard C/C++ allocation primitives.
- `PreferExternal` falls back directly to the same internal/default-local heap because no external provider exists.
- `RequireExternal` fails with `nullptr` without modifying an existing allocation.
- over-aligned allocation uses the standard aligned allocation primitive and remains compatible with `Strata::free`.

### ESP32 backend

The ESP32 backend maps placement to ESP-IDF heap capabilities:

- `Default` → 8-bit capable normal heap behavior;
- `Internal` → internal + 8-bit capable memory;
- external placement → external RAM + 8-bit capable memory.

`PreferExternal` performs two explicit attempts: external first, then internal. `RequireExternal` performs only the external attempt. No PSRAM-specific term leaks into the public Strata API.

Reallocation uses ESP-IDF's capability-aware realloc behavior, which can move an existing allocation when the requested capabilities change.

## FreeRTOS boundary

Strata will eventually provide optional low-level helpers for placing FreeRTOS task stacks and queue backing storage. It must not become a task orchestration framework.

Worker remains responsible for jobs, concurrency, cancellation, lifecycle, cleanup, callbacks, and application-facing asynchronous work. Worker will later depend on Strata for the memory and task-stack mechanics it currently implements itself.

## Safety rules

Allocation and later integration phases must preserve these rules:

1. Required constraints never silently degrade.
2. External memory is never assumed safe for DMA, ISR, flash/cache-disabled, or other platform-sensitive contexts.
3. Platform-specific capabilities stay behind explicit Strata abstractions.
4. Strata has no required global initialization and no hidden mutable global default placement.
5. Failure behavior is deterministic and suitable for embedded builds without requiring exceptions in ordinary APIs.
6. Diagnostics distinguish requested placement from actual memory region.
7. Failed reallocation never consumes or invalidates the caller's original allocation.

## Current boundary

Phase 2 ends at raw allocation mechanics. Region introspection, heap diagnostics, typed construction/destruction, STL allocators, owned buffers, capability constraints, and FreeRTOS integration remain later independent phases.
