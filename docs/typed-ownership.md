# Typed storage and object ownership

Phase 4 adds typed helpers on top of Strata's raw allocation engine without changing the placement contract.

## Raw typed arrays

`allocateArray<T>(count, placement)` allocates aligned raw storage large enough for `count` objects of `T`.

It **does not construct elements** and therefore does not start object lifetimes. The caller must either use the storage only where implicit lifetime rules permit it or explicitly construct objects before use. `Strata::free()` releases this raw storage and does not run element destructors.

The helper returns `nullptr` for a zero count, multiplication overflow, or allocation failure. Alignment is automatically set to `alignof(T)`.

```cpp
auto *storage = Strata::allocateArray<MyRecord>(32, Strata::Placement::PreferExternal);
if (storage != nullptr) {
    // Raw typed storage only. No MyRecord constructors have run.
    Strata::free(storage);
}
```

## Constructed objects

`create<T>(placement, args...)` allocates storage with `alignof(T)` and then constructs exactly one `T` in that storage. `destroy(ptr)` performs the inverse operation: it runs the destructor and then returns the storage to Strata.

```cpp
auto *object = Strata::create<MyType>(Strata::Placement::Internal, arg1, arg2);
if (object != nullptr) {
    use(*object);
    Strata::destroy(object);
}
```

`destroy(nullptr)` is a no-op.

To preserve Strata's deterministic embedded failure model without exception-dependent cleanup, Phase 4 object helpers require constructors and destructors used through these APIs to be non-throwing. Allocation failure is represented by `nullptr`.

## RAII ownership

`Deleter<T>` pairs a destructor call with `Strata::free()`. `UniquePtr<T>` is an alias for `std::unique_ptr<T, Deleter<T>>`, and `makeUnique<T>()` combines allocation, construction, and ownership.

```cpp
auto object = Strata::makeUnique<MyType>(Strata::Placement::PreferExternal, arg1);
if (object) {
    object->run();
}
```

The object is destroyed and its Strata allocation is released automatically when the unique pointer is reset or leaves scope. The ownership type is move-only, matching `std::unique_ptr` semantics.

## Alignment and placement

All typed helpers delegate storage acquisition to the existing `AllocationRequest` path. This means over-aligned object types retain their required alignment and all placement/fallback rules remain unchanged:

- `Internal` requires internal memory;
- `PreferExternal` may fall back to internal memory;
- `RequireExternal` returns an empty/null result if external memory cannot satisfy the request.

Typed ownership does not add an allocation registry, hidden global state, or a second allocator path.
