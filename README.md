# Strata

Strata is a portable C++20 memory placement and allocation library with first-class ESP32 internal-RAM and PSRAM support.

It gives applications and libraries one vocabulary for allocation intent while platform-specific backends own the underlying memory mechanics. Core allocation APIs remain standard-C++ compatible, while optional PMR, FreeRTOS, and ArduinoJson integrations add specialized capabilities without leaking them into the core contract.

[![CI](https://github.com/ZekStack/strata/actions/workflows/ci.yml/badge.svg)](https://github.com/ZekStack/strata/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/ZekStack/strata?sort=semver)](https://github.com/ZekStack/strata/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE.md)

## Why use Strata?

- **Explicit placement** — request default, internal, preferred-external, or required-external memory.
- **Shared memory policy** — embed `Strata::MemoryPolicy` in ZekStack library configs for consistent allocation and task-stack defaults.
- **Portable vocabulary** — application code describes memory intent instead of ESP-IDF heap flags.
- **Strict capability requirements** — DMA and executable requirements never silently degrade.
- **Typed ownership** — raw allocation, typed construction, unique ownership, shared ownership, and move-only buffers use the same placement model.
- **STL integration** — placement-aware allocators and container helpers preserve allocation intent.
- **PMR integration** — use the same placement policies through standard polymorphic allocators and nested PMR containers.
- **Runtime diagnostics** — inspect actual memory regions, heap statistics, and current/peak region usage.
- **Optional advanced diagnostics** — opt into allocation/failure/fallback counters without a global allocation registry.
- **Optional FreeRTOS memory primitives** — explicitly place task stacks and queue item storage, and own static mutex control blocks, while keeping FreeRTOS out of the core headers.
- **Optional ArduinoJson allocation** — route ArduinoJson 7 document memory through the same Strata placement policies.
- **Standalone core** — Strata does not depend on other ZekStack libraries.

## Install

### PlatformIO

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

lib_deps =
    https://github.com/ZekStack/strata.git

build_flags =
    -std=gnu++20
build_unflags =
    -std=gnu++11
```

ArduinoJson users should add ArduinoJson separately because it remains an optional integration:

```ini
lib_deps =
    https://github.com/ZekStack/strata.git
    bblanchon/ArduinoJson@^7.4.3
```

### Arduino IDE

Strata is not published to Arduino Library Manager yet.

Install it by downloading the repository ZIP or cloning it into the Arduino libraries directory:

```text
Arduino/libraries/Strata
```

## Quick start

```cpp
#include <Arduino.h>
#include <Strata.h>

void setup() {
    Serial.begin(115200);

    void *bulk = Strata::allocate(4096, Strata::Placement::PreferExternal);
    if (bulk == nullptr) {
        Serial.println("allocation failed");
        return;
    }

    Serial.printf("region=%u\n", static_cast<unsigned>(Strata::regionOf(bulk)));
    Strata::free(bulk);
}

void loop() {
    delay(1000);
}
```

Owned buffers and STL helpers use the same placement policy:

```cpp
Strata::Buffer buffer(4096, Strata::Placement::PreferExternal);
auto values = Strata::makeVector<int>(Strata::Placement::PreferExternal);
values.push_back(42);
```

Reusable library configs can expose the shared policy type directly:

```cpp
struct SomeLibConfig {
    Strata::MemoryPolicy memory{};
};

SomeLibConfig config;
config.memory.allocation = Strata::Placement::PreferExternal;
config.memory.taskStack = Strata::Placement::Internal;
```

## Important notes

> [!IMPORTANT]
> `PreferExternal` may fall back to internal memory. `RequireExternal` never does. Capability requirements such as DMA or executable memory remain mandatory during fallback.

- `Placement` describes requested policy; `Region` describes where memory actually resides.
- `MemoryPolicy` is local configuration vocabulary, not a global Strata setting. Hard safety requirements may tighten its defaults.
- `Placement::Default` means backend-default allocation and must not be used as an inheritance sentinel.
- Invalid or corrupted `Placement` enum values are rejected by allocation/support APIs instead of being interpreted as another policy.
- Generic builds support normal process-heap allocation but deliberately report external-memory and embedded hardware capabilities as unsupported. The generic backend is validated with GCC/Clang-compatible C++20 toolchains; native MSVC is not supported.
- Raw allocation, typed ownership, and `Buffer` report allocation failure without requiring exceptions.
- `Strata::Allocator<T>` follows standard allocator expectations and throws `std::bad_alloc` when exceptions are enabled.
- The optional PMR adapter requires standard-library `<memory_resource>` support and exceptions to preserve `std::pmr::memory_resource` failure semantics.
- External RAM is not automatically safe for DMA, ISR use, or cache-disabled flash windows.
- FreeRTOS task, queue, and mutex integrations are opt-in and require static allocation support. The task wrapper also requires `INCLUDE_vTaskDelete == 1` and `INCLUDE_uxTaskGetStackHighWaterMark == 1`; the mutex wrapper requires mutex and recursive-mutex support.
- A `Strata::FreeRTOS::Task` owner must be destroyed/reset from a different task context than the task it owns; managed tasks must not self-delete.
- Tasks that can execute while flash/cache is disabled should keep their stacks in internal memory.
- ISR-accessible Strata queues require internal item storage; external queue storage is task-only.
- Strata mutex control blocks are always internal and use FreeRTOS static creation APIs.
- ArduinoJson integration is opt-in, targets ArduinoJson 7, and requires the Strata allocator object to outlive the `JsonDocument` using it.
- Advanced allocation counters are disabled by default; enable them build-wide with `STRATA_ENABLE_ADVANCED_DIAGNOSTICS=1`.

## Examples

| Example | Description |
| --- | --- |
| `Basic` | Minimal allocation, region inspection, and cleanup. |
| `Placement` | Placement and region terminology. |
| `Allocation` | Raw allocation and explicit requests. |
| `Diagnostics` | Support queries and heap statistics. |
| `TypedOwnership` | Typed construction and ownership helpers. |
| `STL` | Placement-aware standard-library containers. |
| `Buffer` | Move-only owned byte buffers and resize behavior. |
| `Capabilities` | DMA/executable capability requirements. |
| `FreeRTOSTask` | Optional placed FreeRTOS task stacks and diagnostics. |
| `FreeRTOSQueue` | Optional typed FreeRTOS queues with placed item storage. |
| `FreeRTOSMutex` | Optional static mutex and recursive-mutex ownership. |
| `ArduinoJson` | Optional ArduinoJson 7 document allocation through Strata placement. |

Start with:

```text
examples/Basic
```

## Documentation

| Document | Description |
| --- | --- |
| [`docs/getting-started.md`](docs/getting-started.md) | Installation, first allocation, and placement choices. |
| [`docs/placement.md`](docs/placement.md) | Stable placement, region, fallback, reallocation, and safety semantics. |
| [`docs/memory-policy.md`](docs/memory-policy.md) | Shared ZekStack library configuration contract for allocations and task stacks. |
| [`docs/migration.md`](docs/migration.md) | Recipes for migrating raw allocation, library policies, containers, tasks, queues, mutexes, and adapters. |
| [`docs/configuration.md`](docs/configuration.md) | Build requirements, backend selection, PSRAM, diagnostics, exceptions, and optional integrations. |
| [`docs/api.md`](docs/api.md) | Public API overview and include boundaries. |
| [`docs/examples.md`](docs/examples.md) | Guide to the included sketches. |
| [`docs/troubleshooting.md`](docs/troubleshooting.md) | Common allocation, PSRAM, capability, and integration issues. |
| [`docs/architecture.md`](docs/architecture.md) | Architectural boundaries, stable vocabulary, platform mapping, and failure contracts. |
| [`docs/diagnostics.md`](docs/diagnostics.md) | Region/heap introspection plus optional allocation/failure/fallback counters. |
| [`docs/typed-ownership.md`](docs/typed-ownership.md) | Typed raw storage, object lifetime, and unique ownership. |
| [`docs/stl.md`](docs/stl.md) | Stateful allocator semantics and STL helpers. |
| [`docs/pmr.md`](docs/pmr.md) | Optional placement-aware `std::pmr::memory_resource` integration. |
| [`docs/buffer.md`](docs/buffer.md) | Owned byte buffers and resize semantics. |
| [`docs/capabilities.md`](docs/capabilities.md) | Required DMA/executable constraints and safety boundaries. |
| [`docs/freertos-tasks.md`](docs/freertos-tasks.md) | Optional task-stack placement and static task creation. |
| [`docs/freertos-queues.md`](docs/freertos-queues.md) | Optional typed queue storage placement and ISR safety. |
| [`docs/freertos-mutexes.md`](docs/freertos-mutexes.md) | Optional internal static mutex and recursive-mutex ownership. |
| [`docs/arduinojson.md`](docs/arduinojson.md) | Optional ArduinoJson 7 custom allocator integration. |
| [`docs/roadmap.md`](docs/roadmap.md) | Completed `v0.1.0` roadmap and post-release planning boundary. |
| [`docs/ecosystem-adoption.md`](docs/ecosystem-adoption.md) | Planned adoption across ZekStack and Core. |
| [`docs/releasing.md`](docs/releasing.md) | Version/tag requirements and the validated GitHub release flow. |

## API overview

```cpp
void *raw = Strata::allocate(4096, Strata::Placement::PreferExternal);
void *dma = Strata::allocate(Strata::AllocationRequest{
    .sizeBytes = 1024,
    .placement = Strata::Placement::Internal,
    .alignment = 32,
    .capabilities = Strata::Capability::Dma,
});

Strata::MemoryPolicy policy{
    .allocation = Strata::Placement::PreferExternal,
    .taskStack = Strata::Placement::Internal,
};

Strata::Buffer buffer(4096, Strata::Placement::PreferExternal);
auto object = Strata::makeUnique<MyType>(Strata::Placement::PreferExternal, constructorArg);
auto values = Strata::makeVector<int>(Strata::Placement::PreferExternal);
auto stats = Strata::memoryStats(Strata::Region::Internal);

Strata::free(raw);
Strata::free(dma);
```

With advanced diagnostics enabled at build time:

```cpp
auto total = Strata::allocationDiagnostics();
auto preferred = Strata::allocationDiagnostics(Strata::Placement::PreferExternal);

Serial.printf("failures=%u fallbacks=%u\n",
    static_cast<unsigned>(total.failures),
    static_cast<unsigned>(preferred.preferredExternalFallbacks));
```

Optional PMR integration:

```cpp
#include <strata/pmr/MemoryResource.h>

Strata::MemoryResource resource{Strata::Placement::PreferExternal};
std::pmr::vector<std::pmr::string> values{&resource};
values.emplace_back("Strata");
```

Optional ArduinoJson integration:

```cpp
#include <ArduinoJson.h>
#include <strata/arduinojson/Allocator.h>

Strata::ArduinoJson::Allocator allocator{Strata::Placement::PreferExternal};
ArduinoJson::JsonDocument document{&allocator};
document["library"] = "Strata";
```

Optional FreeRTOS integrations:

```cpp
#include <strata/freertos/Task.h>
#include <strata/freertos/Queue.h>
#include <strata/freertos/Mutex.h>

Strata::FreeRTOS::Task task = Strata::FreeRTOS::Task::create(worker, nullptr, {
    .name = "worker",
    .stackBytes = 4096,
    .stackPlacement = Strata::Placement::PreferExternal,
    .priority = 1,
    .affinity = Strata::FreeRTOS::NoAffinity,
});

auto queue = Strata::FreeRTOS::Queue<Event>::create({
    .length = 16,
    .storagePlacement = Strata::Placement::PreferExternal,
    .usage = Strata::FreeRTOS::QueueUsage::TaskOnly,
});

auto mutex = Strata::FreeRTOS::Mutex::create();
auto recursiveMutex = Strata::FreeRTOS::RecursiveMutex::create();
```

## Compatibility

| Item | Support |
| --- | --- |
| Language | C++20 |
| Core API | Standard C++ |
| ESP32 backend | Arduino ESP32 / ESP-IDF-compatible build environment |
| Generic backend | GCC/Clang-compatible C++20 process heap; native MSVC is not supported |
| External memory | ESP32 PSRAM through ESP-IDF heap capabilities |
| Core dependencies | none |
| Optional PMR integration | Standard-library `<memory_resource>` with exceptions enabled |
| Optional FreeRTOS integration | FreeRTOS with static allocation; task deletion/high-water-mark APIs for tasks; mutex and recursive-mutex support for mutex wrappers |
| Optional ArduinoJson integration | ArduinoJson 7; CI compatibility target 7.4.3 |
| Advanced diagnostics | Optional compile-time counters; disabled by default |
| Exceptions | Not required by core APIs; STL/PMR standard allocator surfaces follow standard semantics |
| Status | `v0.1.1` compatibility release for ZekStack memory-policy adoption |

## License

MIT — see [`LICENSE.md`](LICENSE.md).

## ZekStack

Part of the ZekStack library stack. `v0.1.1` adds the common consuming-library memory-policy contract and static FreeRTOS mutex ownership so Worker and subsequent libraries can migrate without inventing library-specific placement vocabulary.
