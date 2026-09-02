# Configuration

Strata has no global runtime configuration object. Configuration is expressed through build settings and per-allocation/per-task requests.

## C++ standard

Strata requires C++20. PlatformIO projects should remove the Arduino default C++11 flag and add C++20 explicitly.

```ini
build_flags =
    -std=gnu++20
build_unflags =
    -std=gnu++11
```

## Backend selection

The core library selects its backend at compile time:

- ESP32 builds use ESP-IDF heap-capability APIs for internal/external placement, capability constraints, and diagnostics.
- Generic builds use the normal process heap and intentionally report unsupported embedded capabilities as unavailable.

No caller-side backend object is required.

## External memory

ESP32 external placement requires PSRAM to be available in the active board configuration.

- `PreferExternal` falls back to internal memory when the full request can still be satisfied.
- `RequireExternal` fails instead of degrading.
- `regionOf()` reports where a successful allocation actually resides.

## Alignment and capabilities

`AllocationRequest` combines size, placement, alignment, and required capabilities. Invalid alignment or unsatisfied required capabilities cause allocation failure.

Capability constraints are requirements, not preferences. Placement fallback never removes them.

## Exceptions

Raw allocation, typed ownership, and `Buffer` do not require exceptions for ordinary allocation failure.

`Strata::Allocator<T>` follows the standard allocator contract. With exceptions enabled, allocation failure throws `std::bad_alloc`. With exceptions disabled, it returns `nullptr`; standard containers are not required to recover from that condition.

## FreeRTOS integration

FreeRTOS support is opt-in through `<strata/freertos/Task.h>` and requires static allocation support.

`TaskConfig` configures:

- task name;
- stack bytes;
- stack placement;
- priority;
- affinity.

Use internal stack placement for tasks that may execute while flash/cache is disabled. External stacks are subject to the target's FreeRTOS/ESP-IDF and cache-safety constraints.
