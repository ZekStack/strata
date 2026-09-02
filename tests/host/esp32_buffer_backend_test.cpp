#include <Strata.h>
#include <esp_heap_caps.h>

#include <cassert>
#include <cstdint>

int main() {
    fake_heap_caps_reset();

    Strata::Buffer preferred(32, Strata::Placement::PreferExternal);
    assert(preferred.data() != nullptr);
    assert(preferred.region() == Strata::Region::External);
    auto *bytes = preferred.data<std::uint8_t>();
    bytes[0] = 0xA5;
    bytes[1] = 0x5A;

    fake_heap_caps_fail_external = true;
    assert(preferred.resize(64));
    assert(preferred.size() == 64);
    assert(preferred.placement() == Strata::Placement::PreferExternal);
    assert(preferred.region() == Strata::Region::Internal);
    assert(preferred.data<std::uint8_t>()[0] == 0xA5);
    assert(preferred.data<std::uint8_t>()[1] == 0x5A);

    fake_heap_caps_reset();
    Strata::Buffer required(32, Strata::Placement::RequireExternal);
    assert(required.data() != nullptr);
    assert(required.region() == Strata::Region::External);
    void *original = required.data();
    required.data<std::uint8_t>()[0] = 0x77;

    fake_heap_caps_fail_external = true;
    assert(!required.resize(128));
    assert(required.data() == original);
    assert(required.size() == 32);
    assert(required.placement() == Strata::Placement::RequireExternal);
    assert(required.region() == Strata::Region::External);
    assert(required.data<std::uint8_t>()[0] == 0x77);

    required.reset();
    assert(fake_heap_caps_allocations.empty());
    return 0;
}
