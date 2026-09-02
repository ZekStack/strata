# Placement and fallback

Strata separates requested allocation policy from observed memory location. This distinction is part of the stable public contract.

## Placement is intent

`Strata::Placement` describes what the caller requests:

| Placement | Contract |
| --- | --- |
| `Default` | Use the backend's normal allocation behavior. |
| `Internal` | Require internal/default-local memory. |
| `PreferExternal` | Try external memory first, then fall back to internal memory if the complete request can still be satisfied. |
| `RequireExternal` | Require external memory; fail rather than fall back. |

`PreferExternal` is a preference. `Internal` and `RequireExternal` are requirements.

## Region is observation

`Strata::Region` describes where successful storage actually resides:

- `Unknown` — the backend cannot classify the pointer;
- `Internal` — internal/default-local memory;
- `External` — external memory such as ESP32 PSRAM.

Use `regionOf(ptr)` when actual placement matters after allocation. Never infer the final region from a `PreferExternal` request.

## Fallback matrix

| Request | External available | External allocation fails | External unavailable |
| --- | --- | --- | --- |
| `Default` | Backend-defined | Backend-defined | Backend-defined |
| `Internal` | Internal only | Internal only | Internal only |
| `PreferExternal` | External first | Retry internal | Internal |
| `RequireExternal` | External only | Fail | Fail |

A fallback attempt does not weaken other requirements. Alignment and `Capability` flags remain mandatory on every attempt.

## Capabilities and placement

`AllocationRequest` combines byte count, placement, alignment, and required capabilities. A `PreferExternal` request may fall back internally only if the internal allocation also satisfies every required capability. Strata never drops DMA or executable requirements to make fallback succeed.

## Reallocation

`reallocate(ptr, newSize, placement)` may move storage to satisfy the new placement request.

- `reallocate(nullptr, size, placement)` behaves like a new allocation.
- `reallocate(ptr, 0, placement)` frees the allocation and returns `nullptr`.
- failed reallocation leaves the original allocation valid and owned by the caller.
- `PreferExternal` may migrate between regions while preserving the requested policy.
- `RequireExternal` never retries internally.

The raw reallocation API is for default-aligned allocations. Over-aligned allocations created with `AllocationRequest` must be released with `Strata::free()` rather than passed to `reallocate()`.

## Zero-size behavior

The ordinary raw APIs use deterministic embedded-oriented zero-size semantics: `allocate(0, ...)` and zero-sized `calloc` return `nullptr`, while `free(nullptr)` is a no-op. Adapters that must obey a different external standard may normalize zero-size requests internally; PMR is one example.

## Safety-sensitive contexts

External memory is not automatically safe merely because it is allocatable. Keep storage internal when it may be used from an ISR unless explicitly proven safe, while flash/cache is disabled, by peripherals requiring internal/DMA-capable memory, or by low-level control structures whose platform API requires internal accessibility.

Strata's optional FreeRTOS queue API therefore requires internal storage for ISR-accessible queues, and task stacks that may run during cache-disabled windows should remain internal.

## ESP32 mapping

On ESP32, the backend translates the portable vocabulary to ESP-IDF heap capabilities. Internal placement adds the internal-memory requirement; external placement adds the SPIRAM requirement; DMA/executable capabilities are combined with placement rather than replacing it. `PreferExternal` performs an explicit external attempt followed by internal fallback, while `RequireExternal` performs no fallback.

These ESP-IDF details stay behind Strata. Application and reusable-library APIs should expose `Placement`, `Region`, and `Capability` rather than `MALLOC_CAP_*` or PSRAM-specific enums.
