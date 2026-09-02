#pragma once

#include "Typed.h"

#include <cstddef>
#include <limits>
#include <new>
#include <type_traits>

namespace Strata {
namespace Internal {

template <typename T>
[[nodiscard]] T *allocatorFailure() {
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
    throw std::bad_alloc{};
#else
    return nullptr;
#endif
}

} // namespace Internal

template <typename T>
class Allocator {
    static_assert(std::is_object_v<T> && !std::is_array_v<T>);
    static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>);

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;
    using is_always_equal = std::false_type;

    template <typename U>
    struct rebind {
        using other = Allocator<U>;
    };

    constexpr explicit Allocator(Placement placement = Placement::Default) noexcept
        : placement_(placement) {}

    template <typename U>
    constexpr Allocator(const Allocator<U> &other) noexcept
        : placement_(other.placement()) {}

    [[nodiscard]] constexpr Placement placement() const noexcept {
        return placement_;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    [[nodiscard]] T *allocate(size_type count) {
        if (count == 0) {
            return nullptr;
        }
        if (count > max_size()) {
            return Internal::allocatorFailure<T>();
        }

        auto *ptr = allocateArray<T>(count, placement_);
        return ptr != nullptr ? ptr : Internal::allocatorFailure<T>();
    }

    void deallocate(T *ptr, size_type) noexcept {
        Strata::free(ptr);
    }

    [[nodiscard]] constexpr Allocator select_on_container_copy_construction() const noexcept {
        return *this;
    }

private:
    Placement placement_;
};

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(const Allocator<T> &lhs, const Allocator<U> &rhs) noexcept {
    return lhs.placement() == rhs.placement();
}

} // namespace Strata
