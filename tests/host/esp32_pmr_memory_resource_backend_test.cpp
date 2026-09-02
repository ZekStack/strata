#include <strata/Diagnostics.h>
#include <strata/pmr/MemoryResource.h>

#include <esp_heap_caps.h>

#include <cassert>
#include <cstddef>
#include <memory_resource>
#include <new>
#include <vector>

int main() {
    using Strata::MemoryResource;
    using Strata::Placement;
    using Strata::Region;

    fake_heap_caps_reset();
    {
        MemoryResource preferred{Placement::PreferExternal};
        std::pmr::vector<std::byte> bytes{&preferred};
        bytes.resize(64 * 1024);
        assert(Strata::regionOf(bytes.data()) == Region::External);
    }
    assert(fake_heap_caps_allocations.empty());

    fake_heap_caps_reset();
    fake_heap_caps_fail_external = true;
    {
        MemoryResource fallback{Placement::PreferExternal};
        std::pmr::vector<std::byte> bytes{&fallback};
        bytes.resize(64 * 1024);
        assert(Strata::regionOf(bytes.data()) == Region::Internal);
    }
    assert(fake_heap_caps_allocations.empty());

    fake_heap_caps_reset();
    {
        MemoryResource internal{Placement::Internal};
        std::pmr::vector<std::byte> bytes{&internal};
        bytes.resize(64 * 1024);
        assert(Strata::regionOf(bytes.data()) == Region::Internal);
    }
    assert(fake_heap_caps_allocations.empty());

    fake_heap_caps_reset();
    fake_heap_caps_external_total = 0;
    MemoryResource strict{Placement::RequireExternal};
    bool threw = false;
    try {
        (void)strict.allocate(64 * 1024, alignof(std::max_align_t));
    } catch (const std::bad_alloc &) {
        threw = true;
    }
    assert(threw);
    assert(fake_heap_caps_allocations.empty());
}
