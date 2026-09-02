# Migration guide

Use Strata to replace bespoke memory-placement code without changing subsystem behavior in the same step. Migrate one ownership boundary at a time and measure before and after.

## Library configuration

When a reusable library currently exposes its own memory/PSRAM enum, migrate the library-wide defaults to `Strata::MemoryPolicy`:

```cpp
struct SomeLibConfig {
    Strata::MemoryPolicy memory{};
};
```

Typical semantic mappings are:

```text
old Auto / prefer PSRAM   → Placement::PreferExternal
old Internal              → Placement::Internal
old mandatory PSRAM       → Placement::RequireExternal
```

Preserve the old runtime behavior during the first migration even if a different default may eventually be preferable. Do not create a replacement library-specific placement enum.

For per-operation inheritance, use an explicit optional override. `Placement::Default` means backend-default allocation and must not mean "inherit".

See `memory-policy.md` for the ecosystem-wide configuration contract.

## Raw heap allocation

Before:

```cpp
void *buffer = malloc(size);
```

After:

```cpp
void *buffer = Strata::allocate(size, Strata::Placement::Default);
// ...
Strata::free(buffer);
```

Use `Internal` when storage must remain internal. Use `PreferExternal` for large task-only/general-purpose storage where internal fallback is acceptable. Use `RequireExternal` only when internal fallback would violate the subsystem contract.

## ESP32 PSRAM helpers

Replace `ps_malloc()` or direct `heap_caps_malloc(..., MALLOC_CAP_SPIRAM | ...)` calls with portable Strata placement:

```cpp
void *buffer = Strata::allocate(size, Strata::Placement::PreferExternal);
```

When external memory is mandatory, use `RequireExternal`. When additional hardware properties are mandatory, express them with `AllocationRequest` and `Capability` instead of leaking ESP-IDF flags into reusable code.

## Owned byte buffers

Replace manually paired allocation/free code with `Strata::Buffer` when ownership is singular and byte-oriented:

```cpp
Strata::Buffer buffer{8192, Strata::Placement::PreferExternal};
if (!buffer) {
    // Handle allocation failure.
}
```

`Buffer` is move-only and preserves its requested placement across resize operations.

## Typed objects and arrays

Use `makeUnique<T>()`, `create<T>()`, and typed allocation helpers when object lifetime belongs with the allocation. This removes repeated placement-aware destructor/free code while retaining explicit failure behavior.

## STL containers

Replace custom PSRAM allocators with `Strata::Allocator<T>` or the container factories:

```cpp
auto values = Strata::makeVector<Value>(Strata::Placement::PreferExternal);
auto text = Strata::makeString(Strata::Placement::PreferExternal);
```

Standard containers follow standard allocator failure rules. With exceptions enabled, OOM is reported with `std::bad_alloc`. Exception-disabled standard containers are not required to recover safely; use raw/owned Strata APIs when deterministic no-exception OOM handling is required.

## PMR containers

For polymorphic allocator propagation, opt in to `Strata::MemoryResource`:

```cpp
#include <strata/pmr/MemoryResource.h>

Strata::MemoryResource resource{Strata::Placement::PreferExternal};
std::pmr::vector<std::pmr::string> values{&resource};
```

The resource must outlive every PMR object that refers to it. PMR support requires exceptions because `std::pmr::memory_resource` reports allocation failure with `std::bad_alloc`.

## FreeRTOS task stacks

When replacing custom static-task/PSRAM stack helpers, use `<strata/freertos/Task.h>`. Keep stacks internal for tasks that may execute while flash/cache is disabled. Strata provides low-level memory/task ownership; Worker remains the orchestration layer for jobs, cancellation, retries, pools, and lifecycle policy.

## FreeRTOS queue storage

Use `<strata/freertos/Queue.h>` for typed static queues. Task-only item storage may prefer or require external memory. ISR-accessible queues must remain internal unless a future platform contract explicitly proves another configuration safe.

## FreeRTOS mutexes

Replace dynamically allocated or library-local FreeRTOS mutex owners with `<strata/freertos/Mutex.h>` where the existing lifecycle matches. `Mutex` and `RecursiveMutex` allocate their static control block through Strata as `Internal` and use the static FreeRTOS creation APIs. Do not expose a placement setting for mutex control metadata.

## ArduinoJson

For ArduinoJson 7, keep the dependency optional and provide a Strata allocator explicitly:

```cpp
Strata::ArduinoJson::Allocator allocator{Strata::Placement::PreferExternal};
ArduinoJson::JsonDocument document{&allocator};
```

The allocator must outlive the document.

## Migration checklist

For each subsystem:

1. Record current internal free heap, minimum free heap, largest internal block, external free heap, and relevant task high-water marks.
2. Identify the true safety requirement: internal, external-preferred, external-required, DMA, ISR, or cache-disabled.
3. Replace library-specific placement vocabulary with `MemoryPolicy`/`Placement` where appropriate while preserving current defaults.
4. Replace only the allocation/ownership layer first; preserve behavior and timing.
5. Verify actual region with diagnostics where placement matters.
6. Exercise OOM/fallback paths, cleanup, restart, and shutdown behavior.
7. Measure memory again and compare fragmentation as well as total free bytes.
8. Add a source/CI contract preventing the migrated subsystem from reintroducing direct platform allocation paths.
9. Only then migrate the next storage class.

Do not move flash/cache-disabled, ISR-facing, or DMA-sensitive memory to external RAM merely to increase reported free internal heap.
