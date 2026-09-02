# Roadmap

This document tracks remaining Strata library work. Completed functionality belongs in `CHANGELOG.md`; ecosystem migration work is tracked separately in `ecosystem-adoption.md`.

## Phase 12 — API stability and release hardening

- [ ] Add dedicated placement/fallback documentation if the existing API docs become too dense.
- [ ] Expand ESP32 backend and PSRAM implementation documentation.
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
