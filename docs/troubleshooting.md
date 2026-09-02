# Troubleshooting

## `RequireExternal` returns null

Confirm that PSRAM is present and enabled for the selected board/environment. `RequireExternal` intentionally fails when external memory cannot satisfy the request.

Use `supports(Strata::Placement::RequireExternal)` and the external `MemoryStats` query to distinguish unsupported external memory from ordinary out-of-memory conditions.

## `PreferExternal` returned internal memory

That is the intended fallback contract. Use `regionOf(ptr)` when the actual region matters after allocation. Use `RequireExternal` when fallback is not acceptable.

## Capability allocation fails

DMA and executable flags are hard requirements. The requested placement, alignment, and complete capability set must all be satisfiable at once.

Do not assume that an allocation is safe for a specific peripheral merely because the generic DMA capability is available; peripherals may impose additional alignment, lifetime, or ownership rules.

## Host build reports external memory unsupported

Generic builds deliberately use the normal process heap and do not pretend that host memory has ESP32 PSRAM/DMA/executable semantics. This allows portable contract tests without weakening embedded requirements.

## STL allocation behavior differs with exceptions disabled

`Strata::Allocator<T>` follows standard allocator behavior when exceptions are enabled. With exceptions disabled it cannot throw `std::bad_alloc`; allocation returns `nullptr`, and standard containers are not required to recover safely.

Use raw/typed Strata ownership APIs when deterministic no-exception OOM handling is required.

## FreeRTOS task header fails to compile

The optional task integration requires FreeRTOS static allocation support. Ensure `configSUPPORT_STATIC_ALLOCATION == 1` and include `<strata/freertos/Task.h>` only in FreeRTOS-capable builds.

## External task stack is unsafe during flash work

External memory remains subject to cache-disabled restrictions. Tasks that can run during flash/cache-disabled windows should use `Placement::Internal` for their stack.

## CI metadata validation fails

Keep these versions synchronized:

- `library.json`;
- `library.properties`;
- the matching heading in `CHANGELOG.md`;
- release tags (`vX.Y.Z`) when creating a release.
