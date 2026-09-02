#include <Strata.h>

#include <cassert>
#include <cstdlib>

int main() {
    assert(Strata::regionOf(nullptr) == Strata::Region::Unknown);

    int stackValue = 0;
    assert(Strata::regionOf(&stackValue) == Strata::Region::Unknown);

    void *foreign = std::malloc(32);
    assert(foreign != nullptr);
    assert(Strata::regionOf(foreign) == Strata::Region::Unknown);
    std::free(foreign);

    void *owned = Strata::allocate(64, Strata::Placement::Internal);
    assert(owned != nullptr);
    assert(Strata::regionOf(owned) == Strata::Region::Unknown);
    Strata::free(owned);

    assert(Strata::supports(Strata::Placement::Default));
    assert(Strata::supports(Strata::Placement::Internal));
    assert(Strata::supports(Strata::Placement::PreferExternal));
    assert(!Strata::supports(Strata::Placement::RequireExternal));

    assert(!Strata::supports(Strata::Region::Unknown));
    assert(Strata::supports(Strata::Region::Internal));
    assert(!Strata::supports(Strata::Region::External));

    const auto internal = Strata::memoryStats(Strata::Region::Internal);
    assert(!internal.totalBytes.has_value());
    assert(!internal.freeBytes.has_value());
    assert(!internal.minimumFreeBytes.has_value());
    assert(!internal.largestFreeBlockBytes.has_value());

    const auto external = Strata::memoryStats(Strata::Region::External);
    assert(!external.totalBytes.has_value());
    assert(!external.freeBytes.has_value());
    assert(!external.minimumFreeBytes.has_value());
    assert(!external.largestFreeBlockBytes.has_value());

    return 0;
}
