# Strata

Strata is a lightweight, standalone C++ memory placement and allocation utility library. It gives applications and libraries one portable vocabulary for expressing memory intent while platform-specific backends own the actual memory mechanics.

[![CI](https://github.com/ZekStack/Strata/actions/workflows/ci.yml/badge.svg)](https://github.com/ZekStack/Strata/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> [!NOTE]
> Strata is under active development. Phases 1–5 provide placement, raw allocation, diagnostics, typed ownership, and placement-aware STL allocation. Owned buffers and optional FreeRTOS integration are planned in later phases.

## Design goals

- **Standalone** — Strata has no dependency on other ZekStack libraries.
- **Portable vocabulary** — public APIs describe internal and external memory rather than ESP32-specific PSRAM details.
- **Explicit fallback semantics** — callers can distinguish preference from requirement.
- **Platform-owned mechanics** — ESP-IDF, FreeRTOS, and other platform details stay behind Strata boundaries.
- **Infrastructure, not orchestration** — Strata may provide low-level FreeRTOS memory helpers later, but Worker remains the high-level job/task framework.
- **Deterministic core failure** — raw and typed ownership APIs return null/empty results; the standard allocator adapter follows STL exception semantics when exceptions are enabled.

## Quick start

```cpp
#include <Strata.h>

void *fast = Strata::allocate(256, Strata::Placement::Internal);
void *bulk = Strata::allocate(4096, Strata::Placement::PreferExternal);

Strata::free(fast);
Strata::free(bulk);
```

Placement-aware STL containers use the same memory policy:

```cpp
auto values = Strata::makeVector<int>(Strata::Placement::PreferExternal);
values.push_back(42);

auto text = Strata::makeString(Strata::Placement::Internal);
text = "Strata";

auto shared = Strata::makeShared<int>(Strata::Placement::PreferExternal, 7);
```

## Core APIs

Strata provides raw allocation, diagnostics, typed storage/ownership, and the stateful `Allocator<T>` standard allocator adapter. `Vector<T>` and `String` remain ordinary STL types with a Strata allocator, while `makeVector()`, `makeString()`, and `makeMap()` are convenience factories.

Required placement constraints never silently degrade. `PreferExternal` may fall back to internal memory; `RequireExternal` fails if external memory is unavailable.

## Allocation failure and exceptions

Raw allocation and typed ownership APIs preserve deterministic embedded behavior by returning `nullptr` or an empty smart pointer on ordinary allocation failure.

`Strata::Allocator<T>` follows standard allocator expectations: with exceptions enabled, allocation failure throws `std::bad_alloc`. With exceptions disabled, it returns `nullptr`; standard containers are not required to recover safely from that condition. No-exception code that must handle OOM should therefore prefer the raw/typed Strata APIs rather than STL-container OOM recovery.

## Public terminology

### Placement

| Value | Contract |
| --- | --- |
| `Default` | Use the platform's normal allocator and behavior. |
| `Internal` | Memory must come from the platform's internal/default-local memory region. |
| `PreferExternal` | Prefer external memory and fall back to internal memory if allowed by the platform. |
| `RequireExternal` | External memory is mandatory; the operation fails when it cannot be satisfied. |

### Region

`Region` describes where memory actually resides: `Unknown`, `Internal`, or `External`. Placement is a request; region is an observed result.

## Platform model

- **Generic** — `Default`, `Internal`, and `PreferExternal` use the standard process heap. Because the generic backend has no external-memory provider, `RequireExternal` fails. Pointer regions and non-portable heap statistics remain explicitly unavailable/unknown.
- **ESP32** — placement maps to ESP-IDF heap capabilities, region introspection uses native memory-range helpers, and internal/external heap diagnostics come from capability-specific heap statistics. `PreferExternal` retries internal memory only after the external attempt fails.

ESP32 external memory remains subject to the platform's cache, flash-operation, DMA, ISR, and task-stack restrictions. Strata does not make external memory safe for contexts where the platform itself forbids it.

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

### Arduino IDE

Clone or extract the repository into the Arduino libraries directory:

```text
Arduino/libraries/Strata
```

## Documentation

- [`docs/architecture.md`](docs/architecture.md) — architectural boundaries, allocation semantics, and platform mapping.
- [`docs/diagnostics.md`](docs/diagnostics.md) — region introspection, support queries, and heap statistics.
- [`docs/typed-ownership.md`](docs/typed-ownership.md) — typed raw storage, object lifetime, and unique ownership.
- [`docs/stl.md`](docs/stl.md) — stateful allocator semantics, STL helpers, shared ownership, and failure behavior.
- [`docs/TODO.md`](docs/TODO.md) — complete phased implementation and migration roadmap.

## Compatibility

| Item | Support |
| --- | --- |
| Language | C++20 |
| Core API | Standard C++ |
| ESP32 backend | Arduino ESP32 / ESP-IDF-compatible build environment |
| External memory | ESP32 external RAM through ESP-IDF heap capabilities |
| Dependencies | none |
| Exceptions | raw/typed APIs do not require exceptions; STL allocator throws `std::bad_alloc` when exceptions are enabled |
| Status | Early-stage `0.1.0`; Phase 5 STL allocator support available |

## ZekStack adoption

Strata is designed to become the common memory layer beneath ZekStack libraries such as Worker, Signal, Trace, Tempo, and Passage. Those migrations happen only after Strata's own allocation and platform contracts are stable and independently tested.

## License

MIT — see [`LICENSE`](LICENSE).
