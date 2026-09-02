# Diagnostics and region introspection

Phase 3 adds read-only memory introspection without introducing a global allocation registry or hidden mutable state.

## Region introspection

`Strata::regionOf(ptr)` reports the observed memory region when the active platform can determine it safely.

- `nullptr` always reports `Region::Unknown`.
- ESP32 uses the platform memory-range helpers and reports `Internal` or `External` for pointers in those regions.
- The generic backend reports `Unknown` for arbitrary pointers, including Strata allocations, because standard C++ has no portable API for proving process-heap ownership or physical memory region. Strata intentionally does not add a mandatory allocation registry solely to manufacture this information.

`regionOf()` describes location, not ownership. On platforms with native address-range introspection a pointer does not need to have been allocated by Strata to have a known region.

## Support queries

`supports(Placement)` answers whether a placement policy can currently be satisfied according to its contract. `PreferExternal` is supported when at least its legal fallback path exists; `RequireExternal` is supported only when external memory exists.

`supports(Region)` answers whether the platform currently exposes that physical region. `Region::Unknown` is a result state rather than an allocatable region and therefore is never reported as supported.

On the generic backend, `Internal` is supported and `External` is not. On ESP32 the answers are derived from ESP-IDF heap-capability totals, so external support reflects whether an external heap is actually present.

## Memory statistics

`memoryStats(region)` returns:

- `totalBytes`
- `freeBytes`
- `minimumFreeBytes`
- `largestFreeBlockBytes`

Each field is `std::optional<std::size_t>`. An unavailable statistic is represented by `std::nullopt`; zero is therefore reserved for a real measured value and is never overloaded to mean “unsupported”.

The generic backend returns unavailable fields because the standard library exposes no portable process-heap statistics. ESP32 maps internal and external regions to their respective ESP-IDF heap capabilities and reports the corresponding heap counters.

## Safety and scope

Diagnostics are query-only. They do not change allocation policy, make external memory safe during flash/cache-disabled work, or imply DMA/ISR/task-stack suitability. Those platform-sensitive capability constraints remain separate from basic region diagnostics.
