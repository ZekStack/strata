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

The `v0.1.0` generic backend is validated with GCC/Clang-compatible C++20 toolchains. MSVC is intentionally rejected because its CRT does not provide the `std::aligned_alloc`/`std::free` pairing used by the generic over-aligned allocation contract. Native MSVC support requires a backend with matching aligned-allocation provenance and is outside the `v0.1.0` compatibility contract.

## External memory

ESP32 external placement requires PSRAM to be available in the active board configuration.

- `PreferExternal` falls back to internal memory when the full request can still be satisfied.
- `RequireExternal` fails instead of degrading.
- `regionOf()` reports where a successful allocation actually resides.

## Advanced diagnostics

Advanced allocation/failure/fallback counters are compile-time opt-in and disabled by default. Enable them with:

```ini
build_flags =
    -std=gnu++20
    -DSTRATA_ENABLE_ADVANCED_DIAGNOSTICS=1
```

The setting must be consistent across the Strata library and consuming translation units. When disabled, allocation-path instrumentation is compiled out and diagnostic counter queries return zeros.

Region support queries and `memoryStats()` remain available regardless of this flag. See `diagnostics.md` for counter semantics and overhead details.

## Alignment and capabilities

`AllocationRequest` combines size, placement, alignment, and required capabilities. Invalid placement values, invalid alignment, or unsatisfied required capabilities cause allocation failure.

Capability constraints are requirements, not preferences. Placement fallback never removes them.

## Exceptions

Raw allocation, typed ownership, and `Buffer` do not require exceptions for ordinary allocation failure.

`Strata::Allocator<T>` follows the standard allocator contract. With exceptions enabled, allocation failure throws `std::bad_alloc`. With exceptions disabled, it returns `nullptr`; standard containers are not required to recover from that condition.

The optional PMR adapter requires exceptions because `std::pmr::memory_resource` uses `std::bad_alloc` for allocation failure. This requirement applies only to translation units that include `<strata/pmr/MemoryResource.h>`.

## PMR integration

PMR support is opt-in through `<strata/pmr/MemoryResource.h>` and requires a standard library that provides `<memory_resource>`.

`Strata::MemoryResource` carries one placement policy and forwards PMR allocation size/alignment directly to Strata. Nested PMR-aware containers propagate the resource using standard polymorphic allocator semantics.

Keep the `MemoryResource` object alive for at least as long as every PMR container or allocator that references it.

## FreeRTOS integration

FreeRTOS support is opt-in through `<strata/freertos/Task.h>` and `<strata/freertos/Queue.h>` and requires static allocation support.

The task integration additionally requires `INCLUDE_vTaskDelete == 1` and `INCLUDE_uxTaskGetStackHighWaterMark == 1`. `Task.h` checks these settings at compile time so a FreeRTOS configuration that cannot satisfy the public task API fails with an actionable error instead of failing later on missing symbols.

`TaskConfig` configures task name, stack bytes, stack placement, priority, and affinity. Use internal stack placement for tasks that may execute while flash/cache is disabled.

A `Strata::FreeRTOS::Task` must be reset or destroyed from a different task context than the task it owns. Self-deletion cannot return through the owner cleanup path to release the caller-owned static stack and control block safely. Tasks owned by this wrapper must also not independently call `vTaskDelete(nullptr)`.

Task-only queue item storage may use external memory. ISR-accessible queues require internal item storage.

## ArduinoJson integration

ArduinoJson support is opt-in through `<strata/arduinojson/Allocator.h>`. The header requires ArduinoJson major version 7 to be available but ArduinoJson is not listed as a core Strata dependency.

The CI compatibility target and example builds use ArduinoJson 7.4.3. Applications should include `<ArduinoJson.h>` and keep the `Strata::ArduinoJson::Allocator` object alive for at least as long as every `ArduinoJson::JsonDocument` that references it.
