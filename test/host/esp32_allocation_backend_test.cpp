#include <Strata.h>
#include <esp_heap_caps.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {

void assertInternalCaps(std::uint32_t caps) {
    assert((caps & MALLOC_CAP_INTERNAL) != 0);
    assert((caps & MALLOC_CAP_8BIT) != 0);
    assert((caps & MALLOC_CAP_SPIRAM) == 0);
}

void assertExternalCaps(std::uint32_t caps) {
    assert((caps & MALLOC_CAP_SPIRAM) != 0);
    assert((caps & MALLOC_CAP_8BIT) != 0);
}

} // namespace

int main() {
    fake_heap_caps_reset();
    auto *internal = Strata::allocate(32, Strata::Placement::Internal);
    assert(internal != nullptr);
    assertInternalCaps(fake_heap_caps_last_caps);
    Strata::free(internal);

    fake_heap_caps_reset();
    auto *external = Strata::allocate(32, Strata::Placement::RequireExternal);
    assert(external != nullptr);
    assertExternalCaps(fake_heap_caps_last_caps);
    assert(fake_heap_caps_external_attempts == 1);
    assert(fake_heap_caps_internal_attempts == 0);
    Strata::free(external);

    fake_heap_caps_reset();
    fake_heap_caps_fail_external = true;
    auto *fallback = Strata::allocate(32, Strata::Placement::PreferExternal);
    assert(fallback != nullptr);
    assert(fake_heap_caps_external_attempts == 1);
    assert(fake_heap_caps_internal_attempts == 1);
    assertInternalCaps(fake_heap_caps_last_caps);
    Strata::free(fallback);

    fake_heap_caps_reset();
    fake_heap_caps_fail_external = true;
    assert(Strata::allocate(32, Strata::Placement::RequireExternal) == nullptr);
    assert(fake_heap_caps_external_attempts == 1);
    assert(fake_heap_caps_internal_attempts == 0);

    fake_heap_caps_reset();
    auto *aligned = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 33,
        .placement = Strata::Placement::RequireExternal,
        .alignment = 64,
    });
    assert(aligned != nullptr);
    assert(fake_heap_caps_aligned_attempts == 1);
    assertExternalCaps(fake_heap_caps_last_caps);
    assert(reinterpret_cast<std::uintptr_t>(aligned) % 64 == 0);
    Strata::free(aligned);

    fake_heap_caps_reset();
    fake_heap_caps_fail_external = true;
    auto *zeroed = static_cast<unsigned char *>(
        Strata::calloc(8, sizeof(unsigned char), Strata::Placement::PreferExternal));
    assert(zeroed != nullptr);
    assert(fake_heap_caps_external_attempts == 1);
    assert(fake_heap_caps_internal_attempts == 1);
    for (std::size_t i = 0; i < 8; ++i) {
        assert(zeroed[i] == 0);
    }
    Strata::free(zeroed);

    fake_heap_caps_reset();
    auto *buffer = static_cast<unsigned char *>(Strata::allocate(8, Strata::Placement::Internal));
    assert(buffer != nullptr);
    std::memset(buffer, 0x3C, 8);

    fake_heap_caps_reset();
    fake_heap_caps_fail_external = true;
    auto *migrated = static_cast<unsigned char *>(
        Strata::reallocate(buffer, 32, Strata::Placement::PreferExternal));
    assert(migrated != nullptr);
    assert(fake_heap_caps_external_attempts == 1);
    assert(fake_heap_caps_internal_attempts == 1);
    assertInternalCaps(fake_heap_caps_last_caps);
    for (std::size_t i = 0; i < 8; ++i) {
        assert(migrated[i] == 0x3C);
    }
    Strata::free(migrated);

    fake_heap_caps_reset();
    auto *preserved = static_cast<unsigned char *>(Strata::allocate(8, Strata::Placement::Internal));
    assert(preserved != nullptr);
    std::memset(preserved, 0x7E, 8);

    fake_heap_caps_reset();
    fake_heap_caps_fail_external = true;
    assert(Strata::reallocate(preserved, 32, Strata::Placement::RequireExternal) == nullptr);
    assert(fake_heap_caps_external_attempts == 1);
    assert(fake_heap_caps_internal_attempts == 0);
    for (std::size_t i = 0; i < 8; ++i) {
        assert(preserved[i] == 0x7E);
    }
    Strata::free(preserved);

    return 0;
}
