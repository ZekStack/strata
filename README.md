# Strata

Strata is a lightweight, standalone C++ memory placement and allocation utility library. It gives applications and libraries one portable vocabulary for expressing memory intent while platform-specific backends own the actual memory mechanics.

[![CI](https://github.com/ZekStack/Strata/actions/workflows/ci.yml/badge.svg)](https://github.com/ZekStack/Strata/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> [!NOTE]
> Strata is under active development. Phases 1–7 provide placement, raw allocation, diagnostics, typed ownership, placement-aware STL allocation, owned buffers, and required DMA/executable allocation capabilities. Optional FreeRTOS integration is planned next.

## Design goals

- **Standalone** — Strata has no dependency on other ZekStack libraries.
- **Portable vocabulary** — public APIs describe memory intent instead of leaking ESP32-specific PSRAM details.
- **Explicit fallback semantics** — callers can distinguish preference from requirement.
- **Required capabilities stay required** — DMA/executable constraints never silently disappear during placement fallback.
- **Platform-owned mechanics** — ESP-IDF, FreeRTOS, and other platform details stay behind Strata boundaries.
- **Infrastructure, not orchestration** — Worker remains the high-level job/task framework.
- **Deterministic core failure** — raw and typed ownership APIs return null/empty results; the standard allocator adapter follows STL exception semantics when exceptions are enabled.

## Quick start

```cpp
#include <Strata.h>

void *bulk = Strata::allocate(4096, Strata::Placement::PreferExternal);

void *dma = Strata::allocate(Strata::AllocationRequest{
    .sizeBytes = 1024,
    .placement = Strata::Placement::Internal,
    .alignment = 32,
    .capabilities = Strata::Capability::Dma,
});

Strata::free(bulk);
Strata::free(dma);
```

Owned byte buffers preserve their requested placement across resize:

```cpp
Strata::Buffer buffer(4096, Strata::Placement::PreferExternal);
buffer.resize(8192);
```

Placement-aware STL containers use the same memory policy:

```cpp
auto values = Strata::makeVector<int>(Strata::Placement::PreferExternal);
values.push_back(42);
```

## Core APIs

Strata provides raw allocation, diagnostics, typed storage/ownership, `Buffer`, a stateful `Allocator<T>` standard allocator adapter, and capability-constrained `AllocationRequest` allocations.

Required placement constraints never silently degrade. `PreferExternal` may fall back to internal memory, but any requested capabilities remain mandatory during fallback.

## Capabilities

`Capability` is a bitmask of memory properties that the returned allocation must satisfy:

- `Capability::Dma`
- `Capability::Executable`

Use `supports(Capability)` to query whether the current platform exposes memory matching a complete capability set. On ESP32, these map to `MALLOC_CAP_DMA` and `MALLOC_CAP_EXEC`. Generic builds deliberately report them unsupported rather than pretending the ordinary process heap satisfies embedded hardware constraints.

Capability support is not a substitute for subsystem safety rules. DMA peripherals may impose additional alignment/ownership restrictions, allocation APIs are not automatically ISR-safe, and external RAM remains unsuitable during cache-disabled flash windows.

## Allocation failure and exceptions

Raw allocation, `Buffer`, and typed ownership APIs preserve deterministic embedded behavior by returning failure without requiring exceptions.

`Strata::Allocator<T>` follows standard allocator expectations: with exceptions enabled, allocation failure throws `std::bad_alloc`. With exceptions disabled, it returns `nullptr`; standard containers are not required to recover safely from that condition.

## Public terminology

### Placement

| Value | Contract |
| --- | --- |
| `Default` | Use the platform's normal allocator and behavior. |
| `Internal` | Memory must come from the platform's internal/default-local memory region. |
| `PreferExternal` | Prefer external memory and fall back to internal memory if the full request can still be satisfied. |
| `RequireExternal` | External memory is mandatory; the operation fails when it cannot be satisfied. |

### Region

`Region` describes where memory actually resides: `Unknown`, `Internal`, or `External`. Placement is a request; region is an observed result.

## Platform model

- **Generic** — normal/default/internal allocations use the standard process heap. External placement and embedded hardware capabilities are unsupported.
- **ESP32** — placement and capabilities map to ESP-IDF heap capabilities, region introspection uses native memory-range helpers, and internal/external heap diagnostics come from capability-specific heap statistics.

## Install

### PlatformIO

```ini
lib_deps =
    https://github.com/ZekStack/Strata.git

build_flags =
    -std=gnu++20
build_unflags =
    -std=gnu++11
```

## Documentation

- [`docs/architecture.md`](docs/architecture.md) — architectural boundaries and platform mapping.
- [`docs/diagnostics.md`](docs/diagnostics.md) — region introspection, support queries, and heap statistics.
- [`docs/typed-ownership.md`](docs/typed-ownership.md) — typed raw storage, object lifetime, and unique ownership.
- [`docs/stl.md`](docs/stl.md) — stateful allocator semantics and STL helpers.
- [`docs/buffer.md`](docs/buffer.md) — move-only owned byte buffers and resize semantics.
- [`docs/capabilities.md`](docs/capabilities.md) — required DMA/executable constraints and safety boundaries.
- [`docs/TODO.md`](docs/TODO.md) — complete phased implementation and migration roadmap.

## Compatibility

| Item | Support |
| --- | --- |
| Language | C++20 |
| Core API | Standard C++ |
| ESP32 backend | Arduino ESP32 / ESP-IDF-compatible build environment |
| External memory | ESP32 external RAM through ESP-IDF heap capabilities |
| Dependencies | none |
| Status | Early-stage `0.1.0`; Phase 7 capabilities available |

## ZekStack adoption

Strata is designed to become the common memory layer beneath ZekStack libraries such as Worker, Signal, Trace, Tempo, and Passage. Those migrations happen only after Strata's standalone contracts are stable and independently tested.

## License

MIT — see [`LICENSE`](LICENSE).
