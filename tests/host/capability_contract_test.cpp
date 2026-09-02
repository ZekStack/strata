#include <Strata.h>

#include <cassert>
#include <cstdint>

int main() {
    using Strata::Capability;

    constexpr auto combined = Capability::Dma | Capability::Executable;
    static_assert(Strata::hasCapability(combined, Capability::Dma));
    static_assert(Strata::hasCapability(combined, Capability::Executable));
    static_assert(Strata::validCapabilities(combined));

    assert(Strata::supports(Capability::None));
    assert(!Strata::supports(Capability::Dma));
    assert(!Strata::supports(Capability::Executable));
    assert(!Strata::supports(combined));

    auto *dma = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 128,
        .placement = Strata::Placement::Internal,
        .alignment = Strata::DefaultAlignment,
        .capabilities = Capability::Dma,
    });
    assert(dma == nullptr);

    const auto unknown = static_cast<Capability>(std::uint8_t{0x80});
    assert(!Strata::supports(unknown));
    auto *invalid = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 64,
        .capabilities = unknown,
    });
    assert(invalid == nullptr);
}
