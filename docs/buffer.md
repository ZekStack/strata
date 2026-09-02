# Buffer

`Strata::Buffer` is a move-only RAII owner for untyped byte storage allocated through Strata.

```cpp
Strata::Buffer buffer(4096, Strata::Placement::PreferExternal);
if (!buffer.empty()) {
    auto *bytes = buffer.data<std::uint8_t>();
    bytes[0] = 1;
}
```

## Ownership

`Buffer` owns exactly one allocation. Copying is disabled; move construction and move assignment transfer ownership. Destruction calls `Strata::free()` automatically.

`reset()` releases the current allocation and leaves the buffer empty. `release()` transfers the raw pointer to the caller without freeing it. Both operations preserve the buffer's requested placement so the same object can be resized and reused with the original policy.

## Placement and actual region

`placement()` reports the requested `Placement`. It does not change during resize, reset, or release.

`region()` reports the current actual `Region` using `regionOf(data())`. This matters for `PreferExternal`: a buffer may begin in external memory and later migrate to internal memory if an external resize cannot be satisfied and fallback is permitted.

## Resize semantics

`resize(newSize)` uses the buffer's original requested placement.

- resizing to the current size succeeds without reallocating;
- resizing to zero is equivalent to `reset()` and succeeds;
- growing or shrinking preserves existing bytes according to the underlying reallocation contract;
- successful resize may migrate the allocation between regions when the placement policy permits it;
- failed resize returns `false` and leaves pointer, size, contents, placement, and ownership unchanged.

## Typed access

`data<T>()` is a convenience cast over the raw byte storage. It does not construct `T`, track element count, or manage object lifetimes. Callers remain responsible for ensuring that the buffer size, alignment, and object-lifetime rules are appropriate for `T`.
