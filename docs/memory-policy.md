# Memory policy

`Strata::MemoryPolicy` is the shared configuration vocabulary for memory placement across ZekStack libraries. It is a small value type intended to be embedded in consuming-library configuration objects; it is not global Strata configuration.

```cpp
struct MemoryPolicy {
    Placement allocation{Placement::Default};
    Placement taskStack{Placement::Internal};
};
```

The default keeps ordinary allocations on the backend's normal policy and keeps task stacks internal unless a consuming library deliberately chooses another safe default.

## Standard library configuration shape

ZekStack libraries that own both general-purpose dynamic storage and task stacks should expose the same shape:

```cpp
struct SomeLibConfig {
    Strata::MemoryPolicy memory{};
    // Library-specific configuration...
};
```

A caller can then configure every migrated library with the same vocabulary:

```cpp
SomeLibConfig config;
config.memory.allocation = Strata::Placement::PreferExternal;
config.memory.taskStack = Strata::Placement::PreferExternal;
```

Libraries should not introduce replacement enums such as `Psram`, `HeapType`, `WorkerStackType`, or another spelling of `Auto`. Portable APIs use `Strata::Placement` directly.

## Meaning of the fields

### `allocation`

The default placement for ordinary movable/general-purpose library-owned dynamic storage: buffers, records, task-only queue storage, allocator-backed containers, and similar data whose correctness does not require a stricter region.

### `taskStack`

The default placement for library-owned task stacks when the task is safe to run from the requested region.

Neither field overrides a correctness requirement. A library must tighten placement when an allocation participates in ISR access, cache/flash-disabled execution, DMA, executable memory, or another platform-sensitive path.

## Precedence

Use the most specific safe requirement:

```text
hard correctness/capability requirement
        ↓
per-operation or per-resource override
        ↓
library MemoryPolicy
        ↓
Strata/backend default
```

For example, `config.memory.taskStack = PreferExternal` does not permit a flash/cache-disabled worker task to use external memory. That task must explicitly request `Internal`.

## `Default` never means inherit

`Placement::Default` has one stable meaning: use the backend's normal allocation policy. It must never be overloaded to mean "inherit the parent configuration".

When a child operation optionally overrides a library policy, use an explicit inheritance mechanism such as `std::optional<Strata::Placement>`:

```cpp
struct OperationConfig {
    std::optional<Strata::Placement> stackPlacement{};
};
```

`nullopt` means inherit the library policy. `Placement::Default` means explicitly request Strata's backend default.

## Requested placement and observed region

Configuration and diagnostics must keep intent separate from observation:

```cpp
Strata::Placement requested;
Strata::Region actual;
```

A `PreferExternal` request may legally resolve to `Region::Internal`. `RequireExternal` may not. Do not create diagnostics that reinterpret the actual region as another placement policy.

## Safe constraints win

Every allocation should still flow through Strata after a library is migrated, including allocations that are intentionally fixed to `Internal`.

Typical examples that remain explicitly constrained include:

- ISR-visible control data;
- task stacks that may execute while flash/cache is disabled;
- DMA or executable storage whose capabilities require a specific region;
- FreeRTOS control blocks and synchronization metadata;
- other platform-sensitive control state.

`MemoryPolicy` controls storage that is safe to place according to caller preference. It is not permission to weaken safety requirements.

## Adoption checklist

For each migrated ZekStack library:

1. Add `Strata::MemoryPolicy memory` to the main library configuration where both policy categories apply.
2. Remove library-specific PSRAM/memory-placement enums where compatibility permits.
3. Use `Strata::Placement` for explicit resource/operation overrides.
4. Use an explicit optional value for inheritance; never use `Placement::Default` as an inheritance sentinel.
5. Report requested policy with `Placement` and observed storage with `Region`.
6. Route owned raw memory and buffers through Strata.
7. Route movable STL storage through Strata allocators or PMR where appropriate.
8. Use Strata FreeRTOS ownership primitives where available.
9. Keep ISR/cache-disabled/DMA-sensitive/control storage explicitly constrained.
10. Preserve existing library behavior and defaults during the first migration slice.
11. Measure internal free/minimum heap, largest block, external memory, and task high-water marks before changing defaults.
12. After migration, add source/CI contracts that prevent new direct platform allocation paths from reappearing.

Worker is the first reference migration for this contract.
