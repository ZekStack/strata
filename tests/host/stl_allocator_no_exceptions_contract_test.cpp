#include <Strata.h>

#include <cassert>

int main() {
    Strata::Allocator<int> allocator{Strata::Placement::RequireExternal};
    auto *ptr = allocator.allocate(1);
    assert(ptr == nullptr);

    auto vector = Strata::makeVector<int>(Strata::Placement::Internal);
    assert(vector.empty());
    assert(vector.get_allocator().placement() == Strata::Placement::Internal);
    return 0;
}
