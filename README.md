# Strata

Strata is a lightweight, standalone C++ memory placement and allocation utility library. It gives applications and libraries one portable vocabulary for expressing memory intent while platform-specific backends own the actual memory mechanics.

[![CI](https://github.com/ZekStack/Strata/actions/workflows/ci.yml/badge.svg)](https://github.com/ZekStack/Strata/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> [!NOTE]
> Strata is under active development. Phases 1–4 provide placement, raw allocation, diagnostics, typed storage, and unique ownership primitives. STL allocators and optional FreeRTOS integration are planned in later phases.

## Design goals

- **Standalone** — Strata has no dependency on other ZekStack libraries.
- **Portable vocabulary** — public APIs describe internal and external memory rather than ESP32-specific PSRAM details.
- **Explicit fallback semantics** — callers can distinguish preference from requirement.
- **Platform-owned mechanics** — ESP-IDF, FreeRTOS, and other platform details stay behind Strata boundaries.
- **Infrastructure, not orchestration** — Strata may provide low-level FreeRTOS memory helpers later, but Worker remains the high-level job/task framework.
- **Deterministic failure** — ordinary allocation APIs return null/empty results; they do not require exceptions or abort-on-failure behavior.

## Quick start

```cpp
#include <Strata.h>

void *fast = Strata::allocate(256, Strata::Placement::Internal);
void *bulk = Strata::allocate(4096, Strata::Placement::PreferExternal);

Strata::free(fast);
Strata::free(bulk);
```

Typed ownership uses the same placement rules:

```cpp
struct State {
    explicit State(int value) noexcept : value(value) {}
    ~State() noexcept = default;
    int value;
};

auto state = Strata::makeUnique<State>(Strata::Placement::PreferExternal, 42);
```

## Raw allocation API

```cpp
void *Strata::allocate(std::size_t sizeBytes, Placement placement = Placement::Default) noexcept;
void *Strata::allocate(const AllocationRequest &request) noexcept;
void *Strata::calloc(std::size_t count, std::size_t sizeBytes, Placement placement = Placement::Default) noexcept;
void *Strata::reallocate(void *ptr, std::size_t newSizeBytes, Placement placement = Placement::Default) noexcept;
void Strata::free(void *ptr) noexcept;
```

The raw allocation behavior is deterministic: zero-sized requests return `nullptr`, `calloc` rejects multiplication overflow, failed reallocation leaves the original allocation valid, `free(nullptr)` is a no-op, and required placement constraints never silently degrade.

## Typed storage and ownership

Phase 4 adds:

- `allocateArray<T>(count, placement)` for overflow-safe, correctly aligned **raw typed storage**;
- `create<T>(placement, args...)` and `destroy(ptr)` for single-object lifetime management;
- `Deleter<T>` and `UniquePtr<T>` for Strata-backed unique ownership;
- `makeUnique<T>(placement, args...)` for allocation, construction, and RAII ownership in one operation.

`allocateArray<T>()` does not construct elements and `Strata::free()` does not run array element destructors. Object helpers require non-throwing construction/destruction so ordinary embedded failure remains a null/empty result rather than relying on exception cleanup.

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

- **Generic** — `Default`, `Internal`, and `PreferExternal` use the standard process heap. Because the generic backend has no external-memory provider, `RequireExternal` returns `nullptr`. Pointer regions and non-portable heap statistics remain explicitly unavailable/unknown.
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
- [`docs/typed-ownership.md`](docs/typed-ownership.md) — typed raw storage, object lifetime, and RAII ownership.
- [`docs/TODO.md`](docs/TODO.md) — complete phased implementation and migration roadmap.

## Compatibility

| Item | Support |
| --- | --- |
| Language | C++20 |
| Core API | Standard C++ |
| ESP32 backend | Arduino ESP32 / ESP-IDF-compatible build environment |
| External memory | ESP32 external RAM through ESP-IDF heap capabilities |
| Dependencies | none |
| Exceptions | not required by production APIs; typed lifetime helpers require non-throwing constructors/destructors |
| Status | Early-stage `0.1.0`; Phase 4 typed ownership available |

## ZekStack adoption

Strata is designed to become the common memory layer beneath ZekStack libraries such as Worker, Signal, Trace, Tempo, and Passage. Those migrations happen only after Strata's own allocation and platform contracts are stable and independently tested.

## License

MIT — see [`LICENSE`](LICENSE).
