# Roadmap

This document tracks remaining Strata library work. Completed functionality belongs in `CHANGELOG.md`; ecosystem migration work is tracked separately in `ecosystem-adoption.md`.

## Phase 9 — Optional FreeRTOS queue storage

- [ ] Add an owning typed queue wrapper or low-level queue-storage helper.
- [ ] Allocate queue backing storage through Strata placement policies.
- [ ] Define queue control-structure placement separately from item storage.
- [ ] Expose the underlying `QueueHandle_t` for infrastructure users.
- [ ] Keep ISR-facing external storage unsupported unless platform safety is explicitly proven.
- [ ] Add lifecycle and placement tests.

## Phase 10 — Optional ArduinoJson adapter

- [ ] Add an adapter only when ArduinoJson is present.
- [ ] Keep ArduinoJson out of Strata's required dependencies.
- [ ] Support internal, preferred-external, and required-external JSON allocation.
- [ ] Test large documents, fallback, destruction, and unavailable external memory.
- [ ] Document the supported ArduinoJson major version/API.

## Phase 11 — PMR support

- [ ] Add `Strata::MemoryResource` based on `std::pmr::memory_resource` where supported.
- [ ] Preserve placement policy through PMR allocations.
- [ ] Test nested PMR containers and allocator propagation.
- [ ] Keep PMR optional for callers that only need core allocation APIs.

## Phase 12 — API stability and release hardening

- [ ] Add dedicated placement/fallback documentation if the existing API docs become too dense.
- [ ] Expand ESP32 backend and PSRAM implementation documentation.
- [ ] Add FreeRTOS queue safety documentation after Phase 9.
- [ ] Add migration recipes from raw allocation and bespoke PSRAM allocators.
- [ ] Audit public naming consistency and remove accidental platform leakage.
- [ ] Audit exception/abort assumptions and embedded failure behavior.
- [ ] Treat the resulting API as the stable base for ecosystem migrations.

## Advanced diagnostics

- [ ] Evaluate optional allocation tags/categories.
- [ ] Evaluate opt-in allocation, failure, and fallback counters.
- [ ] Track preferred-external fallback counts when diagnostics are enabled.
- [ ] Evaluate peak allocated bytes per region.
- [ ] Avoid mandatory global allocation registries, locks, or runtime overhead.
- [ ] Ensure diagnostics cannot recurse into the allocator they observe.
