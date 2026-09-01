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

## Platform boundary

The Phase 1 backend boundary distinguishes generic C++ from ESP32 builds without exposing ESP-specific terminology publicly.

The generic backend is the portable baseline. In future phases it will use standard allocation primitives and treat the normal process heap as the internal/default-local region. A required external allocation is unsupported when the platform has no external-memory backend.

The ESP32 backend will map Strata's concepts to ESP-IDF heap capability APIs. PSRAM is therefore an ESP32 backend implementation detail, not a public Strata concept.

## FreeRTOS boundary

Strata will eventually provide optional low-level helpers for placing FreeRTOS task stacks and queue backing storage. It must not become a task orchestration framework.

Worker remains responsible for jobs, concurrency, cancellation, lifecycle, cleanup, callbacks, and application-facing asynchronous work. Worker will later depend on Strata for the memory and task-stack mechanics it currently implements itself.

## Safety rules

Future allocation phases must preserve these rules:

1. Required constraints never silently degrade.
2. External memory is never assumed safe for DMA, ISR, flash/cache-disabled, or other platform-sensitive contexts.
3. Platform-specific capabilities stay behind explicit Strata abstractions.
4. Strata has no required global initialization and no hidden mutable global default placement.
5. Failure behavior must be deterministic and suitable for embedded builds without requiring exceptions in ordinary APIs.
6. Diagnostics distinguish requested placement from actual memory region.

## Phase 1 boundary

Phase 1 intentionally contains no allocation API. It freezes the terminology and platform split first so Phase 2 can implement allocation against a stable contract.
