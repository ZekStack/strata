#include <Strata.h>

#include <cassert>
#include <cstdint>
#include <type_traits>

int main() {
    static_assert(!std::is_copy_constructible_v<Strata::Buffer>);
    static_assert(!std::is_copy_assignable_v<Strata::Buffer>);
    static_assert(std::is_nothrow_move_constructible_v<Strata::Buffer>);
    static_assert(std::is_nothrow_move_assignable_v<Strata::Buffer>);

    Strata::Buffer empty;
    assert(empty.empty());
    assert(empty.size() == 0);
    assert(empty.data() == nullptr);
    assert(empty.placement() == Strata::Placement::Default);
    assert(empty.region() == Strata::Region::Unknown);

    Strata::Buffer buffer(32, Strata::Placement::Internal);
    assert(buffer.size() == 32);
    assert(buffer.data() != nullptr);
    assert(buffer.placement() == Strata::Placement::Internal);

    auto *bytes = buffer.data<std::uint8_t>();
    bytes[0] = 0x12;
    bytes[1] = 0x34;

    assert(buffer.resize(64));
    assert(buffer.size() == 64);
    assert(buffer.data<std::uint8_t>()[0] == 0x12);
    assert(buffer.data<std::uint8_t>()[1] == 0x34);
    assert(buffer.placement() == Strata::Placement::Internal);

    Strata::Buffer moved(std::move(buffer));
    assert(buffer.empty());
    assert(buffer.data() == nullptr);
    assert(moved.size() == 64);
    assert(moved.data<std::uint8_t>()[0] == 0x12);

    Strata::Buffer assigned(8);
    assigned = std::move(moved);
    assert(moved.empty());
    assert(assigned.size() == 64);
    assert(assigned.placement() == Strata::Placement::Internal);

    void *released = assigned.release();
    assert(released != nullptr);
    assert(assigned.empty());
    assert(assigned.data() == nullptr);
    assert(assigned.placement() == Strata::Placement::Internal);
    Strata::free(released);

    assert(assigned.resize(16));
    assert(assigned.size() == 16);
    assert(assigned.placement() == Strata::Placement::Internal);
    assigned.reset();
    assert(assigned.empty());
    assert(assigned.placement() == Strata::Placement::Internal);

    return 0;
}
