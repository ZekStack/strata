# Roadmap

This document tracks remaining Strata library work. The foundational implementation phases are complete; completed functionality belongs in `CHANGELOG.md`, while ecosystem migration work is tracked separately in `ecosystem-adoption.md`.

The Phase 12 API, placement/fallback semantics, platform boundaries, and failure contracts are the stable base for ZekStack ecosystem adoption. Future additions should preserve those contracts unless a deliberate breaking release changes them.

## Advanced diagnostics

- [ ] Evaluate optional allocation tags/categories.
- [ ] Evaluate opt-in allocation, failure, and fallback counters.
- [ ] Track preferred-external fallback counts when diagnostics are enabled.
- [ ] Evaluate peak allocated bytes per region.
- [ ] Avoid mandatory global allocation registries, locks, or runtime overhead.
- [ ] Ensure diagnostics cannot recurse into the allocator they observe.
