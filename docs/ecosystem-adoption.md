# Ecosystem adoption

The Phase 12 Strata API and release contracts are the stable base for ecosystem migrations. Each adoption slice should preserve subsystem behavior and measure internal-RAM impact before and after the change.

Use `migration.md` for mechanical migration patterns and `placement.md` for the placement/fallback contract. Adoption should proceed subsystem by subsystem rather than through a repository-wide allocator rewrite.

## Worker

- Replace Worker-specific stack placement vocabulary with `Strata::Placement` where compatibility permits.
- Route normal job and cleanup-task stack placement through Strata primitives.
- Evaluate cleanup-queue backing storage where cache/ISR safety permits.
- Preserve Worker lifecycle, cancellation, concurrency, callbacks, and cleanup semantics.
- Compare internal heap, minimum heap, largest block, PSRAM use, and stack high-water marks.

## Signal

- Migrate stack placement and task storage.
- Migrate event records, payload arena, subscriptions, match tables, and waiter storage where safe.
- Expose requested/actual placement diagnostics consistently.

## Trace

- Replace Trace-specific internal/prefer-PSRAM/require-PSRAM vocabulary with Strata.
- Migrate bounded log storage and task stacks while preserving realtime/logging safety.

## Remaining ZekStack libraries

Evaluate Tempo, Passage, Pulse, Link, Curier, Phase, Accord, Seal, Fresh, Lingo, and Knot individually. Migrate only where Strata removes meaningful duplication or improves memory placement; do not add dependencies merely for uniformity.

## Core

- Migrate task-only StateSync/WebSocket/notification queue backing where safe.
- Migrate persistent STL structures and large general-purpose buffers where measurements justify it.
- Keep OTA flash/cache-disabled work internal.
- Keep DMA-sensitive display buffers explicitly constrained.
- Keep ISR-facing storage internal unless external placement is explicitly proven safe.
- Measure current/minimum internal heap, largest internal block, external free memory, and task high-water marks before and after each slice.
