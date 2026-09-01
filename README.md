# Strata

Strata is a lightweight, standalone C++ memory placement and allocation utility library. It gives applications and libraries one portable vocabulary for expressing memory intent while platform-specific backends own the actual memory mechanics.

[![CI](https://github.com/ZekStack/Strata/actions/workflows/ci.yml/badge.svg)](https://github.com/ZekStack/Strata/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> [!NOTE]
> Strata is in its foundation phase. The placement contract and platform boundary are available now; allocation APIs begin in Phase 2.

## Design goals

- **Standalone** — Strata has no dependency on other ZekStack libraries.
- **Portable vocabulary** — public APIs describe internal and external memory rather than ESP32-specific PSRAM details.
- **Explicit fallback semantics** — callers can distinguish preference from requirement.
- **Platform-owned mechanics** — ESP-IDF, FreeRTOS, and other platform details stay behind Strata boundaries.
- **Infrastructure, not orchestration** — Strata may provide low-level FreeRTOS memory helpers later, but Worker remains the high-level job/task framework.
- **Observable behavior** — later phases will report requested placement, actual region, and memory diagnostics.

## Public terminology

```cpp
#include <Strata.h>

Strata::Placement placement = Strata::Placement::PreferExternal;
Strata::Region region = Strata::Region::Unknown;
```

### Placement

| Value | Contract |
| --- | --- |
| `Default` | Use the platform's normal allocator and behavior. |
| `Internal` | Memory must come from the platform's internal/default-local memory region. |
| `PreferExternal` | Prefer external memory and fall back to internal memory if allowed by the platform. |
| `RequireExternal` | External memory is mandatory; the operation must fail when it cannot be satisfied. |

### Region

`Region` describes where memory actually resides:

- `Unknown`
- `Internal`
- `External`

Placement is a request. Region is an observed result. Keeping those concepts separate is a core Strata rule.

## Platform model

Phase 1 establishes two backend families:

- **Generic** — portable C++ fallback for platforms without an external-memory concept.
- **ESP32** — ESP32 backend selected when the `ESP32` build macro is present. Later phases will map external memory to ESP32 PSRAM through ESP-IDF capability APIs.

No ESP32-specific memory term is part of the generic public API.

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

- [`docs/architecture.md`](docs/architecture.md) — architectural boundaries and terminology.
- [`docs/TODO.md`](docs/TODO.md) — complete phased implementation and migration roadmap.

## Compatibility

| Item | Support |
| --- | --- |
| Language | C++20 |
| Core API | Standard C++ |
| ESP32 backend | Arduino ESP32 / ESP-IDF-compatible build environment |
| External memory | Platform dependent; ESP32 PSRAM support begins in Phase 2 |
| Dependencies | none |
| Exceptions | not required by Phase 1 production code |
| Status | Early-stage `0.1.0` foundation |

## ZekStack adoption

Strata is designed to become the common memory layer beneath ZekStack libraries such as Worker, Signal, Trace, Tempo, and Passage. Those migrations happen only after Strata's own allocation and platform contracts are stable and independently tested.

## License

MIT — see [`LICENSE`](LICENSE).
