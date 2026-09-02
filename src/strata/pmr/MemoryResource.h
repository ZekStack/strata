#pragma once

#include "../Allocation.h"

#if !__has_include(<memory_resource>)
#error "Strata PMR integration requires standard-library <memory_resource> support"
#endif

#include <cstddef>
#include <memory_resource>

#if !defined(__cpp_exceptions)
#error "Strata PMR integration requires exceptions to preserve std::pmr::memory_resource failure semantics"
#endif

namespace Strata {

class MemoryResource final : public std::pmr::memory_resource {
public:
    explicit MemoryResource(Placement placement = Placement::Default) noexcept : placement_(placement) {}

    [[nodiscard]] Placement placement() const noexcept {
        return placement_;
    }

protected:
    [[nodiscard]] void *do_allocate(std::size_t bytes, std::size_t alignment) override {
        auto *ptr = Strata::allocate(AllocationRequest{
            .sizeBytes = bytes == 0 ? 1 : bytes,
            .placement = placement_,
            .alignment = alignment,
            .capabilities = Capability::None,
        });
        if (ptr != nullptr) {
            return ptr;
        }

        return std::pmr::null_memory_resource()->allocate(bytes, alignment);
    }

    void do_deallocate(void *ptr, std::size_t, std::size_t) override {
        Strata::free(ptr);
    }

    [[nodiscard]] bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override {
        return this == &other;
    }

private:
    Placement placement_{Placement::Default};
};

} // namespace Strata
