#pragma once

#include "Allocation.h"

#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace Strata {

template <typename T>
requires (std::is_object_v<T> && !std::is_array_v<T>)
[[nodiscard]] T *allocateArray(
    std::size_t count,
    Placement placement = Placement::Default) noexcept {
    if (count == 0 || count > (std::numeric_limits<std::size_t>::max() / sizeof(T))) {
        return nullptr;
    }

    return static_cast<T *>(allocate(AllocationRequest{
        .sizeBytes = count * sizeof(T),
        .placement = placement,
        .alignment = alignof(T),
    }));
}

template <typename T, typename... Args>
requires (
    std::is_object_v<T> &&
    !std::is_array_v<T> &&
    !std::is_const_v<T> &&
    !std::is_volatile_v<T> &&
    std::is_nothrow_constructible_v<T, Args &&...>)
[[nodiscard]] T *create(Placement placement, Args &&...args) noexcept {
    auto *storage = allocate(AllocationRequest{
        .sizeBytes = sizeof(T),
        .placement = placement,
        .alignment = alignof(T),
    });
    if (storage == nullptr) {
        return nullptr;
    }

    return std::construct_at(static_cast<T *>(storage), std::forward<Args>(args)...);
}

template <typename T>
requires (
    std::is_object_v<T> &&
    !std::is_array_v<T> &&
    std::is_nothrow_destructible_v<T>)
void destroy(T *ptr) noexcept {
    if (ptr == nullptr) {
        return;
    }

    std::destroy_at(ptr);
    Strata::free(ptr);
}

template <typename T>
struct Deleter {
    void operator()(T *ptr) const noexcept
        requires std::is_nothrow_destructible_v<T> {
        Strata::destroy(ptr);
    }
};

template <typename T>
using UniquePtr = std::unique_ptr<T, Deleter<T>>;

template <typename T, typename... Args>
requires (
    std::is_object_v<T> &&
    !std::is_array_v<T> &&
    !std::is_const_v<T> &&
    !std::is_volatile_v<T> &&
    std::is_nothrow_constructible_v<T, Args &&...> &&
    std::is_nothrow_destructible_v<T>)
[[nodiscard]] UniquePtr<T> makeUnique(Placement placement, Args &&...args) noexcept {
    return UniquePtr<T>{create<T>(placement, std::forward<Args>(args)...)};
}

} // namespace Strata
