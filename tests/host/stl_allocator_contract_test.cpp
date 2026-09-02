#include <Strata.h>

#include <cassert>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

struct SharedState {
    static inline int constructed = 0;
    static inline int destroyed = 0;

    explicit SharedState(int value) : value(value) {
        ++constructed;
    }

    ~SharedState() {
        ++destroyed;
    }

    int value;
};

void testAllocatorStateAndTraits() {
    using Alloc = Strata::Allocator<int>;
    static_assert(!std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value);
    static_assert(!std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value);
    static_assert(!std::allocator_traits<Alloc>::propagate_on_container_swap::value);
    static_assert(!std::allocator_traits<Alloc>::is_always_equal::value);

    const Alloc internal{Strata::Placement::Internal};
    const Alloc preferred{Strata::Placement::PreferExternal};
    assert(internal == Alloc{Strata::Placement::Internal});
    assert(!(internal == preferred));

    const Strata::Allocator<long> rebound{internal};
    assert(rebound.placement() == Strata::Placement::Internal);
    assert(internal.select_on_container_copy_construction().placement() == Strata::Placement::Internal);
}

void testVectorStringAndMap() {
    auto vector = Strata::makeVector<int>(Strata::Placement::Internal);
    vector.push_back(4);
    vector.push_back(8);
    assert(vector.size() == 2);
    assert(vector[1] == 8);
    assert(vector.get_allocator().placement() == Strata::Placement::Internal);

    Strata::String string{Strata::Allocator<char>{Strata::Placement::PreferExternal}};
    string = "strata";
    assert(string == "strata");
    assert(string.get_allocator().placement() == Strata::Placement::PreferExternal);

    auto map = Strata::makeMap<int, int>(Strata::Placement::Internal);
    map.emplace(7, 42);
    assert(map.at(7) == 42);
    assert(map.get_allocator().placement() == Strata::Placement::Internal);
}

void testAssignmentKeepsDestinationPlacement() {
    auto internal = Strata::makeVector<int>(Strata::Placement::Internal);
    auto preferred = Strata::makeVector<int>(Strata::Placement::PreferExternal);
    preferred.push_back(11);

    internal = preferred;
    assert(internal.size() == 1);
    assert(internal[0] == 11);
    assert(internal.get_allocator().placement() == Strata::Placement::Internal);

    internal = std::move(preferred);
    assert(internal.size() == 1);
    assert(internal.get_allocator().placement() == Strata::Placement::Internal);
}

void testSharedPlacementAndLifetime() {
    SharedState::constructed = 0;
    SharedState::destroyed = 0;
    {
        auto shared = Strata::makeShared<SharedState>(Strata::Placement::Internal, 73);
        assert(shared);
        assert(shared->value == 73);
        assert(SharedState::constructed == 1);
        assert(SharedState::destroyed == 0);
    }
    assert(SharedState::destroyed == 1);
}

void testExceptionEnabledFailure() {
    bool allocatorThrew = false;
    try {
        Strata::Allocator<int> allocator{Strata::Placement::RequireExternal};
        (void)allocator.allocate(1);
    } catch (const std::bad_alloc &) {
        allocatorThrew = true;
    }
    assert(allocatorThrew);

    bool sharedThrew = false;
    try {
        (void)Strata::makeShared<int>(Strata::Placement::RequireExternal, 5);
    } catch (const std::bad_alloc &) {
        sharedThrew = true;
    }
    assert(sharedThrew);
}

} // namespace

int main() {
    testAllocatorStateAndTraits();
    testVectorStringAndMap();
    testAssignmentKeepsDestinationPlacement();
    testSharedPlacementAndLifetime();
    testExceptionEnabledFailure();
    return 0;
}
