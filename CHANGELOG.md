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
- Add host contracts and ESP32 example builds across ESP32, ESP32-S3, ESP32-C3, and ESP32-P4 targets.
- Add ZekStack-standard repository documentation, tooling, test layout, metadata validation, and release workflow structure.
