#include <Strata.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace {

struct Tracker {
    static inline int constructed = 0;
    static inline int destroyed = 0;

    explicit Tracker(int initial) noexcept : value(initial) {
        ++constructed;
    }

    ~Tracker() noexcept {
        ++destroyed;
    }

    int value;
};

struct MoveOnly {
    explicit MoveOnly(int initial) noexcept : value(initial) {}
    MoveOnly(const MoveOnly &) = delete;
    MoveOnly &operator=(const MoveOnly &) = delete;
    MoveOnly(MoveOnly &&) noexcept = default;
    MoveOnly &operator=(MoveOnly &&) noexcept = default;
    ~MoveOnly() noexcept = default;

    int value;
};

struct alignas(64) AlignedObject {
    explicit AlignedObject(std::uint32_t initial) noexcept : value(initial) {}
    ~AlignedObject() noexcept = default;

    std::uint32_t value;
};

struct TwoBytes {
    std::uint16_t value;
};

static_assert(!std::is_copy_constructible_v<Strata::UniquePtr<MoveOnly>>);
static_assert(std::is_move_constructible_v<Strata::UniquePtr<MoveOnly>>);

} // namespace

int main() {
    Tracker::constructed = 0;
    Tracker::destroyed = 0;

    auto *rawTrackers = Strata::allocateArray<Tracker>(4, Strata::Placement::Internal);
    assert(rawTrackers != nullptr);
    assert(Tracker::constructed == 0);
    Strata::free(rawTrackers);
    assert(Tracker::destroyed == 0);

    assert(Strata::allocateArray<int>(0) == nullptr);
    const auto overflowingCount = (std::numeric_limits<std::size_t>::max() / sizeof(TwoBytes)) + 1;
    assert(Strata::allocateArray<TwoBytes>(overflowingCount) == nullptr);

    auto *tracker = Strata::create<Tracker>(Strata::Placement::Internal, 41);
    assert(tracker != nullptr);
    assert(tracker->value == 41);
    assert(Tracker::constructed == 1);
    assert(Tracker::destroyed == 0);
    Strata::destroy(tracker);
    assert(Tracker::destroyed == 1);
    Strata::destroy<Tracker>(nullptr);

    auto *aligned = Strata::create<AlignedObject>(Strata::Placement::Internal, 77U);
    assert(aligned != nullptr);
    assert(reinterpret_cast<std::uintptr_t>(aligned) % alignof(AlignedObject) == 0);
    assert(aligned->value == 77U);
    Strata::destroy(aligned);

    auto first = Strata::makeUnique<MoveOnly>(Strata::Placement::Default, 123);
    assert(first);
    assert(first->value == 123);
    auto second = std::move(first);
    assert(!first);
    assert(second);
    assert(second->value == 123);

    {
        auto ownedTracker = Strata::makeUnique<Tracker>(Strata::Placement::Internal, 88);
        assert(ownedTracker);
        assert(Tracker::constructed == 2);
        assert(ownedTracker->value == 88);
    }
    assert(Tracker::destroyed == 2);

    assert(Strata::allocateArray<int>(8, Strata::Placement::RequireExternal) == nullptr);
    assert(Strata::create<Tracker>(Strata::Placement::RequireExternal, 5) == nullptr);
    auto unavailable = Strata::makeUnique<Tracker>(Strata::Placement::RequireExternal, 6);
    assert(!unavailable);
    assert(Tracker::constructed == 2);

    return 0;
}
