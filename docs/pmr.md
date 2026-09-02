# PMR

Strata provides an optional `std::pmr::memory_resource` adapter for code that uses C++ polymorphic allocators.

```cpp
#include <strata/pmr/MemoryResource.h>
```

`Strata::MemoryResource` stores one `Strata::Placement` policy and applies it to every allocation requested through the resource.

```cpp
Strata::MemoryResource resource{Strata::Placement::PreferExternal};

std::pmr::vector<std::pmr::string> values{&resource};
values.emplace_back("alpha");
values.emplace_back("beta");
```

PMR allocator propagation means nested PMR-aware containers created through a parent polymorphic allocator continue using the same resource.

## Placement

- `Placement::Internal` keeps PMR allocations internal.
- `Placement::PreferExternal` prefers external memory and may fall back to internal memory.
- `Placement::RequireExternal` requires external memory and reports allocation failure when it cannot be satisfied.
- `Placement::Default` follows the active Strata backend's default allocation policy.

The resource forwards both allocation size and alignment to `Strata::allocate(AllocationRequest)`.

## Failure semantics

`std::pmr::memory_resource` uses the standard allocator failure contract: an allocation that cannot be satisfied throws `std::bad_alloc`.

For that reason, `<strata/pmr/MemoryResource.h>` requires:

- standard-library `<memory_resource>` support;
- exceptions enabled for the translation unit.

This requirement is limited to the optional PMR header. Core Strata allocation APIs remain usable without exceptions.

## Lifetime

A `MemoryResource` must outlive every PMR container or allocator that references it.

The resource does not track allocations globally. Deallocation is forwarded directly to `Strata::free()`, and resource equality is identity-based.

## Include boundary

PMR support is not included by `Strata.h`. Applications that do not use PMR do not compile or depend on `<memory_resource>` through Strata.
