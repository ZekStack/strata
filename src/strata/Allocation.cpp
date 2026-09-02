#include "Allocation.h"

#include "internal/Platform.h"

#include <limits>

namespace Strata {
namespace {

[[nodiscard]] constexpr bool isValidAlignment(std::size_t alignment) noexcept {
    return alignment != 0 && (alignment & (alignment - 1)) == 0;
}

[[nodiscard]] constexpr bool multiplyWouldOverflow(std::size_t lhs, std::size_t rhs) noexcept {
    return rhs != 0 && lhs > (std::numeric_limits<std::size_t>::max() / rhs);
}

} // namespace

void *allocate(std::size_t sizeBytes, Placement placement) noexcept {
    return allocate(AllocationRequest{
        .sizeBytes = sizeBytes,
        .placement = placement,
        .alignment = DefaultAlignment,
        .capabilities = Capability::None,
    });
}

void *allocate(const AllocationRequest &request) noexcept {
    if (
        request.sizeBytes == 0 ||
        !isValidAlignment(request.alignment) ||
        !validCapabilities(request.capabilities)) {
        return nullptr;
    }

    return Internal::allocate(
        request.sizeBytes,
        request.alignment,
        request.placement,
        request.capabilities);
}

void *calloc(std::size_t count, std::size_t sizeBytes, Placement placement) noexcept {
    if (count == 0 || sizeBytes == 0 || multiplyWouldOverflow(count, sizeBytes)) {
        return nullptr;
    }

    return Internal::calloc(count, sizeBytes, placement);
}

void *reallocate(void *ptr, std::size_t newSizeBytes, Placement placement) noexcept {
    if (newSizeBytes == 0) {
        Internal::free(ptr);
        return nullptr;
    }

    if (ptr == nullptr) {
        return allocate(newSizeBytes, placement);
    }

    return Internal::reallocate(ptr, newSizeBytes, placement);
}

void free(void *ptr) noexcept {
    Internal::free(ptr);
}

} // namespace Strata
