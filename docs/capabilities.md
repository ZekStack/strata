# Allocation capabilities

Phase 7 adds required allocation capabilities to `AllocationRequest`. Capabilities describe properties the returned memory must satisfy; they are never best-effort hints.

## Public capabilities

- `Capability::Dma` — memory must come from a region the platform allocator marks DMA-capable.
- `Capability::Executable` — memory must come from a region the platform allocator marks executable.

Capabilities can be combined with `operator|`. Unknown capability bits are rejected.

```cpp
Strata::AllocationRequest request{
    .sizeBytes = 1024,
    .placement = Strata::Placement::Internal,
    .alignment = 32,
    .capabilities = Strata::Capability::Dma,
};

void *ptr = Strata::allocate(request);
```

`supports(Capability)` reports whether the current platform exposes memory satisfying the complete requested capability set. It does not guarantee that a particular allocation size is currently available.

## Placement interaction

Placement and capabilities are both requirements except for `PreferExternal`, whose region preference may fall back to internal memory. Capability requirements are preserved during that fallback. For example, `PreferExternal + Dma` may fall back to internal DMA-capable memory, but it will never fall back to ordinary non-DMA memory.

`RequireExternal + Dma` and `RequireExternal + Executable` fail on ESP32 targets where external RAM does not expose those heap capabilities.

## ESP32 mapping

Strata maps `Dma` to `MALLOC_CAP_DMA` and `Executable` to `MALLOC_CAP_EXEC`. Executable allocations do not additionally require `MALLOC_CAP_8BIT`, because ESP-IDF executable memory may be word-access-only. Normal and DMA allocations remain byte-addressable.

## Safety boundaries

A heap capability is not a blanket safety guarantee:

- DMA-capable memory can still have peripheral-specific alignment, size, ownership, cache, and lifetime requirements.
- Executable memory does not make arbitrary bytes valid code and does not replace instruction-cache or architecture-specific synchronization requirements.
- Allocation/free APIs are not ISR-safe merely because the returned memory is internal or DMA-capable.
- External RAM remains unsafe while its cache is unavailable; capability requests do not make PSRAM usable during flash/cache-disabled windows.
- `Placement::Internal` only constrains physical placement. Callers performing flash operations, ISR work, DMA, or cache-disabled execution must still obey the platform subsystem's rules.

Strata intentionally does not expose vague capabilities such as `IsrSafe` or `FlashSafe`, because those properties depend on execution context and subsystem behavior rather than only on the allocated memory region.
