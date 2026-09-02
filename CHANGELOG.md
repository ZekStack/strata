# Changelog

All notable changes to Strata are documented in this file.

## 0.1.0

- Add portable `Placement` and `Region` vocabulary with generic and ESP32 backends.
- Add raw allocation, calloc, reallocation, explicit allocation requests, alignment, and strict fallback semantics.
- Add region introspection, support queries, and heap diagnostics.
- Add typed allocation, construction, destruction, unique ownership, and shared ownership helpers.
- Add placement-aware STL allocators, aliases, and container factories.
- Add move-only owned `Buffer` storage with placement-preserving resize semantics.
- Add required DMA and executable allocation capabilities.
- Add optional FreeRTOS task-stack ownership, static task creation, byte-normalized stack diagnostics, and explicit task lifetime ownership.
- Add optional typed FreeRTOS queues with placement-aware item storage, internal control blocks, and explicit ISR-safe internal-only mode.
- Add optional ArduinoJson 7 allocator integration with Strata placement, fallback, and strict-external semantics.
- Add optional PMR `MemoryResource` integration with placement-aware polymorphic allocation and nested-container propagation.
- Harden the stable API boundary with dedicated placement/fallback and migration documentation, refreshed architecture docs, core no-exception/public-API contracts, and CI audits against platform leakage and abort/terminate failure policies.
- Add opt-in advanced allocation diagnostics with attempt/success/failure/invalid-request counters, requested-byte accounting, per-placement snapshots, preferred-external fallback counts, and reset support.
- Add current and peak region-used byte diagnostics derived from platform heap free/minimum-free watermarks without an allocation registry.
- Keep advanced diagnostics disabled by default with no allocation-path instrumentation in the default build and fixed atomic state when enabled.
- Reject invalid `Placement` enum values consistently across allocation and support APIs while keeping them out of per-placement diagnostic buckets.
- Require the FreeRTOS task APIs used by Strata at compile time and document that managed task teardown must occur from a different task context.
- Define the `v0.1.0` generic-backend compatibility boundary as GCC/Clang-compatible C++20 toolchains and explicitly reject unsupported native MSVC builds.
- Normalize repository metadata and documentation links to the canonical `ZekStack/strata` URL.
- Add host contracts and ESP32 example builds across ESP32, ESP32-S3, ESP32-C3, and ESP32-P4 targets.
- Add ZekStack-standard repository documentation, tooling, test layout, metadata validation, and release workflow structure.
- Complete the `v0.1.0` implementation roadmap and harden tag-driven GitHub release creation with validated, non-empty changelog notes.
