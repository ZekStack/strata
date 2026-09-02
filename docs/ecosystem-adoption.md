# Ecosystem adoption

Strata `v0.1.1` defines the shared memory-policy contract for ZekStack migrations. Each adoption slice should preserve subsystem behavior and measure internal-RAM impact before and after the change.

Use `memory-policy.md` for the standard configuration shape, `migration.md` for mechanical migration patterns, and `placement.md` for the placement/fallback contract. Adoption should proceed subsystem by subsystem rather than through a repository-wide allocator rewrite.

## Standard contract

Migrated libraries should use `Strata::MemoryPolicy` for library-wide defaults, `Strata::Placement` for resource-specific overrides, and `Strata::Region` for observed-location diagnostics. Library-specific PSRAM enums should disappear where compatibility permits.

Hard safety requirements always override configurable defaults. `Placement::Default` remains backend-default allocation and is never an inheritance sentinel.

## Worker — reference migration

Worker is the first `MemoryPolicy` adopter and should become the reference implementation for subsequent ZekStack libraries.

- Replace `WorkerStackType` and other Worker-specific placement vocabulary with `Strata::Placement`.
- Add `Strata::MemoryPolicy memory` to `WorkerConfig` while preserving Worker's current stack behavior during migration.
- Use an explicit optional per-job stack placement so inheritance is distinct from `Placement::Default`.
- Route normal job and cleanup-task stack ownership through `Strata::FreeRTOS::Task`.
- Replace the library-local recursive mutex owner with `Strata::FreeRTOS::RecursiveMutex`.
- Evaluate cleanup-queue backing storage and migrate it through `Strata::FreeRTOS::Queue` where cache/ISR safety permits.
- Route Worker-owned records, buffers, and movable container storage through Strata.
- Preserve Worker lifecycle, cancellation, concurrency, callbacks, and cleanup semantics.
- Report requested stack placement separately from observed stack region.
- Compare internal heap, minimum heap, largest block, external-memory use, and stack high-water marks.
- Add source contracts preventing direct ESP-IDF heap/task allocation paths from reappearing after migration.

## Signal

- Adopt the same `MemoryPolicy` configuration shape.
- Migrate stack placement and task storage.
- Migrate event records, payload arena, subscriptions, match tables, and waiter storage where safe.
- Expose requested/actual placement diagnostics consistently.

## Trace

- Adopt the same `MemoryPolicy` configuration shape.
- Replace Trace-specific internal/prefer-PSRAM/require-PSRAM vocabulary with Strata.
- Migrate bounded log storage and task stacks while preserving realtime/logging safety.

## Remaining ZekStack libraries

Evaluate Tempo, Passage, Pulse, Link, Curier, Phase, Accord, Seal, Fresh, Lingo, and Knot individually. Use the same `MemoryPolicy` shape wherever a library owns configurable general allocations and/or task stacks. Migrate only where Strata removes meaningful duplication or improves memory placement; do not weaken safety requirements merely for uniformity.

## Core

- Migrate task-only StateSync/WebSocket/notification queue backing where safe.
- Migrate persistent STL structures and large general-purpose buffers where measurements justify it.
- Keep OTA flash/cache-disabled work internal.
- Keep DMA-sensitive display buffers explicitly constrained.
- Keep ISR-facing storage internal unless external placement is explicitly proven safe.
- Measure current/minimum internal heap, largest internal block, external free memory, and task high-water marks before and after each slice.
