# STL allocator and containers

Phase 5 adds a stateful standard allocator and placement-aware STL helpers.

## `Allocator<T>`

`Strata::Allocator<T>` stores a `Placement`. Rebinding the allocator preserves that placement, including the internal rebinding performed by node containers and `std::allocate_shared()`.

Allocator equality is based on requested placement. Two allocators compare equal only when their placement values match. Copy assignment, move assignment, and swap do not propagate allocators, so an existing destination container keeps its own placement policy.

```cpp
Strata::Allocator<int> internal{Strata::Placement::Internal};
Strata::Allocator<int> external{Strata::Placement::PreferExternal};
```

## Containers

`Vector<T>` and `String` are transparent aliases of standard STL types using `Strata::Allocator`:

```cpp
auto values = Strata::makeVector<int>(Strata::Placement::PreferExternal);
auto text = Strata::makeString(Strata::Placement::Internal);
auto table = Strata::makeMap<int, int>(Strata::Placement::PreferExternal);
```

The factories exist to avoid repeating allocator types and constructors. They do not wrap or replace the STL containers.

## Shared ownership

`makeShared<T>()` uses `std::allocate_shared()` with a Strata allocator. The allocator is rebound by the standard library for its combined object/control-block allocation while preserving the requested placement.

```cpp
auto state = Strata::makeShared<State>(Strata::Placement::PreferExternal, 42);
```

## Allocation failure

Standard allocator semantics differ from Strata's raw allocation API:

- with C++ exceptions enabled, `Allocator<T>::allocate()` throws `std::bad_alloc` when Strata cannot satisfy the allocation;
- with exceptions disabled, `Allocator<T>::allocate()` returns `nullptr` because throwing is unavailable;
- standard containers generally assume allocator failure is reported by exception and are not required to recover safely from a null allocator result;
- therefore no-exception code that must handle OOM deterministically should use Strata's raw allocation, typed storage, `create()`, or `makeUnique()` APIs rather than depending on STL-container OOM recovery.

This exception behavior is confined to the standard allocator adapter. The raw Strata allocation APIs continue to return `nullptr` on ordinary allocation failure.
