# Strata

Strata is a lightweight, standalone C++ memory placement and allocation utility library. It gives applications and libraries one portable vocabulary for expressing memory intent while platform-specific backends own the actual memory mechanics.

[![CI](https://github.com/ZekStack/Strata/actions/workflows/ci.yml/badge.svg)](https://github.com/ZekStack/Strata/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> [!NOTE]
> Strata is under active development. Phase 2 provides the raw allocation engine; typed ownership, diagnostics, STL allocators, and optional FreeRTOS integration are planned in later phases.

## Design goals

- **Standalone** — Strata has no dependency on other ZekStack libraries.
- **Portable vocabulary** — public APIs describe internal and external memory rather than ESP32-specific PSRAM details.
- **Explicit fallback semantics** — callers can distinguish preference from requirement.
- **Platform-owned mechanics** — ESP-IDF, FreeRTOS, and other platform details stay behind Strata boundaries.
- **Infrastructure, not orchestration** — Strata may provide low-level FreeRTOS memory helpers later, but Worker remains the high-level job/task framework.
- **Deterministic failure** — ordinary allocation APIs return `nullptr`; they do not require exceptions or abort-on-failure behavior.

## Quick start

```cpp
#include <Strata.h>

void *fast = Strata::allocate(256, Strata::Placement::Internal);
void *bulk = Strata::allocate(4096, Strata::Placement::PreferExternal);

Strata::free(fast);
Strata::free(bulk);
```

For explicit alignment, use `AllocationRequest`:

```cpp
Strata::AllocationRequest request{
    .sizeBytes = 1024,
    .placement = Strata::Placement::PreferExternal,
    .alignment = 64,
};

void *aligned = Strata::allocate(request);
Strata::free(aligned);
```

## Raw allocation API

```cpp
void *Strata::allocate(std::size_t sizeBytes, Placement placement = Placement::Default) noexcept;
void *Strata::allocate(const AllocationRequest &request) noexcept;
void *Strata::calloc(std::size_t count, std::size_t sizeBytes, Placement placement = Placement::Default) noexcept;
void *Strata::reallocate(void *ptr, std::size_t newSizeBytes, Placement placement = Placement::Default) noexcept;
void Strata::free(void *ptr) noexcept;
```

The Phase 2 behavior is intentionally deterministic:

- zero-byte `allocate` and zero-count/zero-element-size `calloc` return `nullptr`;
- `calloc` rejects size multiplication overflow;
- alignment must be a non-zero power of two; unsupported alignments fail with `nullptr`;
- `reallocate(nullptr, size, placement)` behaves like `allocate(size, placement)`;
- `reallocate(ptr, 0, placement)` frees `ptr` and returns `nullptr`;
- failed `reallocate` leaves the original allocation valid;
- `free(nullptr)` is a no-op;
- over-aligned allocations from `AllocationRequest` must currently be released with `Strata::free`; passing them to `Strata::reallocate` is not supported in Phase 2 because portable reallocators do not preserve arbitrary alignment metadata.

## Public terminology

### Placement

| Value | Contract |
| --- | --- |
| `Default` | Use the platform's normal allocator and behavior. |
| `Internal` | Memory must come from the platform's internal/default-local memory region. |
| `PreferExternal` | Prefer external memory and fall back to internal memory if allowed by the platform. |
| `RequireExternal` | External memory is mandatory; the operation fails when it cannot be satisfied. |

### Region

`Region` describes where memory actually resides:

- `Unknown`
- `Internal`
- `External`

Placement is a request. Region is an observed result. Region introspection arrives in Phase 3.

## Platform model

- **Generic** — `Default`, `Internal`, and `PreferExternal` use the standard process heap. Because the generic backend has no external-memory provider, `RequireExternal` returns `nullptr`.
- **ESP32** — `Default` maps to the normal 8-bit capable heap, `Internal` requires internal 8-bit capable memory, and external placement maps to ESP-IDF's external-memory heap capability. `PreferExternal` retries against internal memory only after the external attempt fails.

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
- [`docs/TODO.md`](docs/TODO.md) — complete phased implementation and migration roadmap.

## Compatibility

| Item | Support |
| --- | --- |
| Language | C++20 |
| Core API | Standard C++ |
| ESP32 backend | Arduino ESP32 / ESP-IDF-compatible build environment |
| External memory | ESP32 external RAM through ESP-IDF heap capabilities |
| Dependencies | none |
| Exceptions | not required by production allocation APIs |
| Status | Early-stage `0.1.0`; raw allocation engine available |

## ZekStack adoption

Strata is designed to become the common memory layer beneath ZekStack libraries such as Worker, Signal, Trace, Tempo, and Passage. Those migrations happen only after Strata's own allocation and platform contracts are stable and independently tested.

## License

MIT — see [`LICENSE`](LICENSE).
