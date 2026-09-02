#include <strata/pmr/MemoryResource.h>

#include <cassert>
#include <cstddef>
#include <memory_resource>
#include <string>
#include <vector>

int main() {
    Strata::MemoryResource resource{Strata::Placement::Internal};
    assert(resource.placement() == Strata::Placement::Internal);

    std::pmr::vector<std::pmr::string> values{&resource};
    values.emplace_back("alpha");
    values.emplace_back(2048, 'x');

    assert(values.get_allocator().resource() == &resource);
    assert(values[0].get_allocator().resource() == &resource);
    assert(values[1].get_allocator().resource() == &resource);
    assert(values[0] == "alpha");
    assert(values[1].size() == 2048);

    std::pmr::vector<std::pmr::vector<int>> nested{&resource};
    nested.emplace_back();
    nested.back().resize(1024, 7);

    assert(nested.get_allocator().resource() == &resource);
    assert(nested.back().get_allocator().resource() == &resource);
    assert(nested.back().size() == 1024);
    assert(nested.back().front() == 7);

    void *zero = resource.allocate(0, alignof(std::max_align_t));
    assert(zero != nullptr);
    resource.deallocate(zero, 0, alignof(std::max_align_t));

    Strata::MemoryResource other{Strata::Placement::Internal};
    assert(resource.is_equal(resource));
    assert(!resource.is_equal(other));
}
