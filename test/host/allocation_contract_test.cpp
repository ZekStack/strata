#include <Strata.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

int main() {
    static_assert(Strata::DefaultAlignment == alignof(std::max_align_t));

    assert(Strata::allocate(0) == nullptr);
    assert(Strata::calloc(0, 16) == nullptr);
    assert(Strata::calloc(16, 0) == nullptr);
    assert(Strata::calloc(std::numeric_limits<std::size_t>::max(), 2) == nullptr);

    assert(Strata::allocate(Strata::AllocationRequest{
               .sizeBytes = 64,
               .placement = Strata::Placement::Default,
               .alignment = 0,
           }) == nullptr);
    assert(Strata::allocate(Strata::AllocationRequest{
               .sizeBytes = 64,
               .placement = Strata::Placement::Default,
               .alignment = 3,
           }) == nullptr);

    auto *aligned = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 65,
        .placement = Strata::Placement::Internal,
        .alignment = 64,
    });
    assert(aligned != nullptr);
    assert(reinterpret_cast<std::uintptr_t>(aligned) % 64 == 0);
    Strata::free(aligned);

    auto *zeroed = static_cast<unsigned char *>(Strata::calloc(32, sizeof(unsigned char)));
    assert(zeroed != nullptr);
    for (std::size_t i = 0; i < 32; ++i) {
        assert(zeroed[i] == 0);
    }
    Strata::free(zeroed);

    auto *buffer = static_cast<unsigned char *>(Strata::allocate(16));
    assert(buffer != nullptr);
    for (std::size_t i = 0; i < 16; ++i) {
        buffer[i] = static_cast<unsigned char>(i + 1);
    }

    auto *resized = static_cast<unsigned char *>(Strata::reallocate(buffer, 64));
    assert(resized != nullptr);
    for (std::size_t i = 0; i < 16; ++i) {
        assert(resized[i] == static_cast<unsigned char>(i + 1));
    }

    assert(Strata::reallocate(resized, 0) == nullptr);

    auto *preferred = Strata::allocate(32, Strata::Placement::PreferExternal);
    assert(preferred != nullptr);
    Strata::free(preferred);

    assert(Strata::allocate(32, Strata::Placement::RequireExternal) == nullptr);
    assert(Strata::calloc(2, 16, Strata::Placement::RequireExternal) == nullptr);

    auto *preserved = static_cast<unsigned char *>(Strata::allocate(8));
    assert(preserved != nullptr);
    std::memset(preserved, 0x5A, 8);
    assert(Strata::reallocate(preserved, 32, Strata::Placement::RequireExternal) == nullptr);
    for (std::size_t i = 0; i < 8; ++i) {
        assert(preserved[i] == 0x5A);
    }
    Strata::free(preserved);

    Strata::free(nullptr);
    return 0;
}
