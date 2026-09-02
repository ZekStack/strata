# Diagnostics and region introspection

Strata provides two diagnostic layers: always-available platform/region introspection and optional advanced allocation counters.

## Region introspection

`Strata::regionOf(ptr)` reports the observed memory region when the active platform can determine it safely.

- `nullptr` always reports `Region::Unknown`.
- ESP32 uses the platform memory-range helpers and reports `Internal` or `External` for pointers in those regions.
- The generic backend reports `Unknown` for arbitrary pointers, including Strata allocations, because standard C++ has no portable API for proving process-heap ownership or physical memory region.

`regionOf()` describes location, not ownership. On platforms with native address-range introspection a pointer does not need to have been allocated by Strata to have a known region.

## Support queries

`supports(Placement)` answers whether a placement policy can currently be satisfied according to its contract. `PreferExternal` is supported when at least its legal fallback path exists; `RequireExternal` is supported only when external memory exists.

`supports(Region)` answers whether the platform currently exposes that physical region. `Region::Unknown` is a result state rather than an allocatable region and therefore is never reported as supported.

On the generic backend, `Internal` is supported and `External` is not. On ESP32 the answers are derived from ESP-IDF heap-capability totals, so external support reflects whether an external heap is actually present.

## Memory statistics

`memoryStats(region)` returns optional platform heap values:

- `totalBytes`
- `freeBytes`
- `minimumFreeBytes`
- `largestFreeBlockBytes`
- `usedBytes`
- `peakUsedBytes`

`usedBytes` is derived as `totalBytes - freeBytes`. `peakUsedBytes` is derived as `totalBytes - minimumFreeBytes`, so it represents the platform heap high-water usage for that region rather than memory owned only by Strata.

Each field is `std::optional<std::size_t>`. An unavailable statistic is represented by `std::nullopt`; zero is reserved for a real measured value and is never overloaded to mean “unsupported”.

The generic backend returns unavailable fields because the standard library exposes no portable process-heap statistics. ESP32 maps internal and external regions to their respective ESP-IDF heap capabilities.

## Advanced allocation counters

Advanced counters are disabled by default. Enable them for the entire build with:

```text
-DSTRATA_ENABLE_ADVANCED_DIAGNOSTICS=1
```

The macro must be applied to Strata's compiled sources as well as the calling translation units. PlatformIO `build_flags` naturally applies it to the library build.

Use `advancedDiagnosticsEnabled()` or the `AdvancedDiagnosticsEnabled` constant to verify the active build mode.

`allocationDiagnostics()` returns totals across every placement. `allocationDiagnostics(placement)` returns the same counters for one requested placement policy. `resetAllocationDiagnostics()` clears all counters.

`AllocationDiagnostics` contains:

- `attempts` — allocation/calloc/reallocation attempts, including invalid allocation requests;
- `successes` — attempts that returned storage;
- `failures` — attempts that returned no storage;
- `invalidRequests` — failures rejected before reaching the platform allocator;
- `preferredExternalFallbacks` — successful `PreferExternal` requests that resolved to `Region::Internal`;
- `requestedBytes` — representable bytes requested by attempts;
- `successfulBytes` — requested bytes associated with successful attempts;
- `failedBytes` — requested bytes associated with failed attempts.

A zero-size `reallocate` keeps its established “free and return null” semantics and is not counted as an allocation attempt. A multiplication-overflow `calloc` is counted as an invalid attempt, but contributes zero requested bytes because the requested byte total is not representable.

These byte counters describe request traffic, not current Strata-owned live bytes. Reallocation success contributes its requested new size.

## Concurrency and overhead

When advanced diagnostics are disabled, the allocation path does not update diagnostic state. The query functions remain present and return zeroed counters, which keeps the public API stable across build configurations.

When enabled, Strata uses a fixed set of relaxed atomic counters. There is no allocation registry, dynamic diagnostic memory, global initialization requirement, Strata-owned lock, or callback that could recurse into the allocator. Snapshots are individually atomic but are not a transactional view across every field while other tasks are allocating concurrently.

Counters are `std::size_t` and may wrap after enough cumulative events/bytes. Long-running diagnostic sessions can call `resetAllocationDiagnostics()` at an application-defined boundary.

## Allocation tags/categories

`v0.1.0` intentionally does not add allocation tags or named categories. A built-in fixed taxonomy would impose application-specific meaning on a low-level library, while arbitrary dynamic labels would require registry/storage policy that conflicts with the no-registry diagnostic boundary.

If ecosystem migration later proves that categories are necessary, they can be added as an opt-in fixed-ID extension without changing the existing counters or allocation semantics.

## Safety and scope

Diagnostics do not change allocation policy, make external memory safe during flash/cache-disabled work, or imply DMA/ISR/task-stack suitability. Required capability and placement constraints remain authoritative.
