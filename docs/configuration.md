# Configuration

Strata has no global runtime configuration object. Configuration is expressed through build settings and per-allocation/per-task requests.

`Strata::MemoryPolicy` does not change that rule. It is a reusable value object for embedding in consuming-library configuration structures so ZekStack libraries can expose consistent `allocation` and `taskStack` policy. It never configures Strata globally.

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

The generic backend is validated with GCC/Clang-compatible C++20 toolchains. MSVC is intentionally rejected because its CRT does not provide the `std::aligned_alloc`/`std::free` pairing used by the generic over-aligned allocation contract. Native MSVC support requires a backend with matching aligned-allocation provenance and is outside the current compatibility contract.

## External memory

ESP32 external placement requires PSRAM to be available in the active board configuration.

- `PreferExternal` falls back to internal memory when the full request can still be satisfied.
- `RequireExternal` fails instead of degrading.
- `regionOf()` reports where a successful allocation actually resides.

## Memory policy for consuming libraries

The shared configuration shape is:

```cpp
Strata::MemoryPolicy memory;
memory.allocation = Strata::Placement::PreferExternal;
memory.taskStack = Strata::Placement::Internal;
```

`allocation` is the default for ordinary movable library-owned dynamic storage. `taskStack` is the default for library-owned task stacks. Safety/capability requirements may always tighten those choices. `Placement::Default` retains its backend-default meaning and must not be used as an inheritance sentinel. See `memory-policy.md` for the ecosystem contract.

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

FreeRTOS support is opt-in through `<strata/freertos/Task.h>`, `<strata/freertos/Queue.h>`, `<strata/freertos/Mutex.h>`, and `<strata/freertos/BinarySemaphore.h>` and requires static allocation support.

The task integration additionally requires `INCLUDE_vTaskDelete == 1` and `INCLUDE_uxTaskGetStackHighWaterMark == 1`. `Task.h` checks these settings at compile time so a FreeRTOS configuration that cannot satisfy the public task API fails with an actionable error instead of failing later on missing symbols.

The mutex integration additionally requires `configUSE_MUTEXES == 1` and `configUSE_RECURSIVE_MUTEXES == 1`. Mutex control storage is always internal and is allocated through Strata before using FreeRTOS static creation APIs.

The binary-semaphore integration only adds the static-allocation requirement. Its `StaticSemaphore_t` control storage is always internal, newly created semaphores start empty, and the wrapper provides both task-context and ISR take/give operations. There is intentionally no placement configuration for semaphore control storage.

`TaskConfig` configures task name, stack bytes, stack placement, priority, and affinity. Use internal stack placement for tasks that may execute while flash/cache is disabled.

A `Strata::FreeRTOS::Task` must be reset or destroyed from a different task context than the task it owns. Self-deletion cannot return through the owner cleanup path to release the caller-owned static stack and control block safely. Tasks owned by this wrapper must also not independently call `vTaskDelete(nullptr)`.

Task-only queue item storage may use external memory. ISR-accessible queues require internal item storage.

## ArduinoJson integration

ArduinoJson support is opt-in through `<strata/arduinojson/Allocator.h>`. The header requires ArduinoJson major version 7 to be available but ArduinoJson is not listed as a core Strata dependency.

The CI compatibility target and example builds use ArduinoJson 7.4.3. Applications should include `<ArduinoJson.h>` and keep the `Strata::ArduinoJson::Allocator` object alive for at least as long as every `ArduinoJson::JsonDocument` that references it.
