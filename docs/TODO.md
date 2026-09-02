# Strata implementation roadmap

This document is the implementation checklist for Strata itself and the later ZekStack/Core adoption work. Each phase should leave the repository buildable, documented, and independently reviewable.

## Phase 1 — Repository foundation and public contract

- [x] Establish `ZekStack/Strata` as a standalone library with no ZekStack dependencies.
- [x] Standardize on C++20.
- [x] Add PlatformIO and Arduino library metadata.
- [x] Add the umbrella include `Strata.h`.
- [x] Define `Strata::Placement` with `Default`, `Internal`, `PreferExternal`, and `RequireExternal`.
- [x] Define `Strata::Region` with `Unknown`, `Internal`, and `External`.
- [x] Document exact placement/fallback semantics.
- [x] Add generic and ESP32 backend selection stubs behind an internal platform boundary.
- [x] Add a minimal placement example.
- [x] Add host contract tests for public terminology and backend selection.
- [x] Add CI for source audit, host tests, and ESP32 example builds.
- [x] Document architecture and the Worker/FreeRTOS boundary.

## Phase 2 — Raw allocation engine

- [x] Add `allocate(sizeBytes, placement)`.
- [x] Add `calloc(count, size, placement)` with overflow-safe multiplication.
- [x] Add `reallocate(ptr, newSize, placement)` with documented migration semantics.
- [x] Add `free(ptr)`.
- [x] Introduce an extensible `AllocationRequest` containing size, placement, and alignment.
- [x] Implement generic backend allocation through standard C/C++ allocation primitives.
- [x] Implement ESP32 internal allocation through ESP-IDF heap capabilities.
- [x] Implement ESP32 external/PSRAM allocation as the backend for external placement.
- [x] Implement `PreferExternal` fallback to internal memory.
- [x] Ensure `RequireExternal` fails rather than degrading.
- [x] Define zero-size allocation behavior.
- [x] Define alignment validation and failure behavior.
- [x] Add host allocation/fallback tests.
- [x] Add ESP32 internal/external placement tests.

## Phase 3 — Region introspection and diagnostics

- [ ] Add `regionOf(ptr)`.
- [ ] Add `supports(Placement)` and `supports(Region)` queries.
- [ ] Add `MemoryStats` with total, free, minimum-free, and largest-free-block values.
- [ ] Represent unavailable statistics explicitly rather than with ambiguous zeros.
- [ ] Implement ESP32 internal heap diagnostics.
- [ ] Implement ESP32 external heap diagnostics.
- [ ] Add tests for foreign pointers and unknown regions.
- [ ] Add diagnostics documentation and example.

## Phase 4 — Typed objects and RAII primitives

- [ ] Add overflow-safe `allocateArray<T>()`.
- [ ] Define whether array allocation constructs objects or provides raw typed storage; keep the behavior explicit.
- [ ] Add `create<T>(placement, args...)` using placement construction.
- [ ] Add `destroy<T>()`.
- [ ] Add `Deleter<T>` and Strata-owned unique-pointer support.
- [ ] Add `makeUnique<T>()`.
- [ ] Test constructors, destructors, move-only objects, alignment, and OOM behavior.

## Phase 5 — Standard allocator and STL support

- [ ] Implement stateful `Strata::Allocator<T>`.
- [ ] Define allocator equality and propagation traits correctly for differing placements.
- [ ] Validate vector/string/map-compatible allocator behavior.
- [ ] Add `Vector<T>` and `String` convenience aliases only where they remain transparent STL types.
- [ ] Add ergonomic container factories if allocator construction is otherwise too verbose.
- [ ] Add `makeShared<T>()` via `std::allocate_shared()` so object and control block share placement intent.
- [ ] Explicitly document/test allocation failure behavior with exceptions enabled and disabled.
- [ ] Add STL examples and host tests.

## Phase 6 — Owned `Buffer`

- [ ] Add move-only RAII `Buffer`.
- [ ] Add size, data, typed-data, placement, and actual-region accessors.
- [ ] Add `resize()` while preserving the original requested placement.
- [ ] Add `reset()` and `release()`.
- [ ] Test move construction/assignment, resize migration, failed resize, and cleanup.
- [ ] Add buffer example.

## Phase 7 — Advanced allocation capabilities

- [ ] Add an extensible `Capability` bitmask only for constraints with clear portable semantics.
- [ ] Add capability requirements to `AllocationRequest`.
- [ ] Map DMA/executable constraints to ESP-IDF heap capabilities where supported.
- [ ] Ensure required capabilities never silently degrade.
- [ ] Document cache-disabled, DMA, ISR, and other platform-sensitive restrictions.
- [ ] Add capability support queries and tests.

## Phase 8 — Optional FreeRTOS task memory integration

### Phase 8A — Task stack abstraction

- [ ] Add an optional FreeRTOS integration header without making FreeRTOS a core dependency.
- [ ] Add a task-stack allocation abstraction that always uses byte sizes publicly.
- [ ] Record requested placement and actual stack region.
- [ ] Ensure correct allocation/deallocation APIs are paired on ESP32.

### Phase 8B — Minimal task creation primitive

- [ ] Add a minimal `TaskConfig` for name, stack bytes, placement, priority, and affinity.
- [ ] Add low-level task creation that honors stack placement.
- [ ] Add task stack high-water-mark diagnostics in bytes.
- [ ] Keep jobs, callbacks, retry, cancellation, pools, and orchestration out of Strata.
- [ ] Document Worker as the preferred high-level asynchronous job API.
- [ ] Test internal, preferred-external, required-external, and unavailable-external task creation.

## Phase 9 — Optional FreeRTOS queue storage

- [ ] Add an owning typed queue wrapper or equivalent low-level queue-storage helper.
- [ ] Allocate queue backing storage through Strata placement policies.
- [ ] Decide/document placement of the FreeRTOS queue control structure separately from item backing storage.
- [ ] Expose the underlying `QueueHandle_t` for infrastructure users.
- [ ] Keep ISR-facing queues internal unless explicit platform safety is proven.
- [ ] Add task-only external queue tests and lifecycle tests.

## Phase 10 — Optional ArduinoJson adapter

- [ ] Add an adapter that is available only when ArduinoJson is present.
- [ ] Keep ArduinoJson out of Strata's required dependencies.
- [ ] Support internal, preferred-external, and required-external JSON allocations.
- [ ] Test large documents, destruction, fallback, and external-memory-unavailable cases.
- [ ] Document supported ArduinoJson major version/API.

## Phase 11 — PMR support

- [ ] Add `Strata::MemoryResource` based on `std::pmr::memory_resource` where supported.
- [ ] Preserve placement policy through PMR allocations.
- [ ] Test nested PMR containers and allocator propagation.
- [ ] Keep PMR optional for callers that only need the basic allocator.

## Phase 12 — Documentation, examples, and API stability pass

- [ ] Expand README with stable installation and quick-start examples.
- [ ] Add dedicated placement/fallback documentation.
- [ ] Add ESP32 backend and PSRAM implementation documentation.
- [ ] Add FreeRTOS task/queue safety documentation.
- [ ] Add migration recipes from raw allocation and bespoke PSRAM allocators.
- [ ] Add examples for allocation, buffer, STL, diagnostics, FreeRTOS, and ArduinoJson.
- [ ] Audit public naming consistency and remove accidental platform leakage.
- [ ] Audit source for exception/abort assumptions and embedded failure behavior.
- [ ] Establish release/tag workflow and changelog policy.
- [ ] Treat the resulting API as the stable base for ecosystem migrations.

---

# Ecosystem adoption

These phases begin only after Strata's standalone implementation has proven stable.

## Phase 13 — Migrate Worker

- [ ] Add Strata as Worker's memory-layer dependency.
- [ ] Replace Worker-specific stack placement types with `Strata::Placement` where compatibility permits.
- [ ] Route normal job stack creation through Strata FreeRTOS primitives.
- [ ] Route cleanup-task stack placement through Strata.
- [ ] Route cleanup queue backing storage through Strata where cache/ISR safety permits.
- [ ] Preserve Worker job lifecycle, cancellation, concurrency, callbacks, and cleanup semantics.
- [ ] Compare before/after internal heap, minimum heap, largest block, PSRAM usage, and stack HWM.

## Phase 14 — Migrate Signal

- [ ] Replace Signal stack-placement vocabulary with Strata.
- [ ] Migrate task stack placement.
- [ ] Migrate event records, payload arena, dispatch payload, subscriptions, match table, and waiter storage where safe.
- [ ] Expose requested/actual placement diagnostics consistently.
- [ ] Measure internal-RAM and fragmentation impact.

## Phase 15 — Migrate Trace

- [ ] Replace Trace-specific internal/prefer-PSRAM/require-PSRAM storage vocabulary with Strata.
- [ ] Migrate normal and realtime storage allocators.
- [ ] Migrate Trace task stack placement.
- [ ] Preserve bounded storage semantics and logging safety.
- [ ] Measure the expected large internal-RAM reduction.

## Phase 16 — Migrate remaining ZekStack libraries one by one

- [ ] Tempo — service/worker/dedicated task placement and metadata.
- [ ] Passage — allocator and task-stack placement.
- [ ] Pulse — timer metadata and command queue.
- [ ] Link — buffers, metadata, and JSON allocations.
- [ ] Curier — queued jobs, endpoint/payload storage, JSON/JWT memory.
- [ ] Phase — persistent dependency graph allocation.
- [ ] Accord — table/storage consistency where beneficial.
- [ ] Seal — replace placement vocabulary where it reduces duplication.
- [ ] Fresh — migrate allocation mechanics without conflating the separate storage-task architecture work.
- [ ] Lingo — migrate only where Strata reduces real duplication.
- [ ] Knot — evaluate; do not migrate merely for uniformity if there is no memory benefit.

## Phase 17 — Core adoption

- [ ] Migrate task-only StateSync/WebSocket/notification queue backing where safe.
- [ ] Migrate persistent STL structures with measurable internal-memory benefit.
- [ ] Migrate large JSON and general-purpose buffers.
- [ ] Keep OTA flash/cache-disabled work internal.
- [ ] Keep DMA-sensitive display buffers explicitly constrained.
- [ ] Keep ISR-facing storage internal unless explicitly proven safe.
- [ ] Measure current/minimum internal heap, largest internal block, external free memory, and task HWMs before and after each slice.

## Phase 18 — Advanced diagnostics and profiling

- [ ] Evaluate optional allocation tags/categories.
- [ ] Evaluate opt-in allocation/failure/fallback counters.
- [ ] Track preferred-external fallback counts when diagnostics are enabled.
- [ ] Evaluate peak allocated bytes per region.
- [ ] Avoid mandatory global allocation registries, locks, or runtime overhead.
- [ ] Ensure diagnostics cannot recurse into the allocator they are observing.
