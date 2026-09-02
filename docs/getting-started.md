# Getting started

Strata provides a portable memory-placement vocabulary with platform-specific allocation backends. The core API is standard C++20; ESP32 builds add internal-RAM, PSRAM, capability, and heap-diagnostic support.

## Requirements

- C++20.
- For ESP32: Arduino ESP32 / ESP-IDF-compatible build environment.
- PSRAM must be enabled by the board/platform configuration before external placement can succeed.
- Optional FreeRTOS task integration requires `configSUPPORT_STATIC_ALLOCATION == 1`.

## Install with PlatformIO

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

lib_deps =
    https://github.com/ZekStack/Strata.git

build_flags =
    -std=gnu++20
build_unflags =
    -std=gnu++11
```

## First allocation

```cpp
#include <Strata.h>

void *memory = Strata::allocate(4096, Strata::Placement::PreferExternal);
if (memory == nullptr) {
    // Handle allocation failure.
}

Strata::Region region = Strata::regionOf(memory);
Strata::free(memory);
```

`PreferExternal` means external memory is preferred but internal memory is an allowed fallback. Use `RequireExternal` when that fallback would violate the caller's contract.

## Placement choices

| Placement | Behavior |
| --- | --- |
| `Default` | Use the platform's normal allocator behavior. |
| `Internal` | Allocation must come from internal/default-local memory. |
| `PreferExternal` | Try external memory first and fall back to internal memory. |
| `RequireExternal` | External memory is mandatory; fail if unavailable. |

`Region` reports the observed location of an allocation: `Unknown`, `Internal`, or `External`.

## Typed and owned memory

Use `Strata::Buffer` for move-only byte storage, typed helpers for object ownership, or `Strata::Allocator<T>`/container factories for STL storage.

```cpp
Strata::Buffer buffer(8192, Strata::Placement::PreferExternal);
auto values = Strata::makeVector<int>(Strata::Placement::PreferExternal);
```

## Optional FreeRTOS tasks

FreeRTOS support is not included by `Strata.h`. Opt in explicitly:

```cpp
#include <strata/freertos/Task.h>
```

This keeps the core library usable in standard C++ builds without FreeRTOS headers.

See `examples/Basic` first, then use the specialized examples for allocation, diagnostics, STL, buffers, capabilities, and task stacks.
