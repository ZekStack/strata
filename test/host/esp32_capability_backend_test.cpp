#include <Strata.h>
#include <esp_heap_caps.h>

#include <cassert>

int main() {
    using Strata::Capability;
    using Strata::Placement;
    using Strata::Region;

    fake_heap_caps_reset();

    assert(Strata::supports(Capability::Dma));
    assert(Strata::supports(Capability::Executable));
    assert(!Strata::supports(Capability::Dma | Capability::Executable));

    auto *dma = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 256,
        .placement = Placement::Internal,
        .alignment = Strata::DefaultAlignment,
        .capabilities = Capability::Dma,
    });
    assert(dma != nullptr);
    assert((fake_heap_caps_last_caps & MALLOC_CAP_DMA) != 0);
    assert((fake_heap_caps_last_caps & MALLOC_CAP_INTERNAL) != 0);
    assert(Strata::regionOf(dma) == Region::Internal);
    Strata::free(dma);

    auto *fallback = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 256,
        .placement = Placement::PreferExternal,
        .alignment = Strata::DefaultAlignment,
        .capabilities = Capability::Dma,
    });
    assert(fallback != nullptr);
    assert(fake_heap_caps_external_attempts == 1);
    assert(fake_heap_caps_internal_attempts == 1);
    assert(Strata::regionOf(fallback) == Region::Internal);
    Strata::free(fallback);

    auto *strictExternal = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 128,
        .placement = Placement::RequireExternal,
        .alignment = Strata::DefaultAlignment,
        .capabilities = Capability::Dma,
    });
    assert(strictExternal == nullptr);

    auto *executable = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 128,
        .placement = Placement::Internal,
        .alignment = Strata::DefaultAlignment,
        .capabilities = Capability::Executable,
    });
    assert(executable != nullptr);
    assert((fake_heap_caps_last_caps & MALLOC_CAP_EXEC) != 0);
    assert((fake_heap_caps_last_caps & MALLOC_CAP_8BIT) == 0);
    Strata::free(executable);

    auto *incompatible = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 128,
        .placement = Placement::Internal,
        .alignment = Strata::DefaultAlignment,
        .capabilities = Capability::Dma | Capability::Executable,
    });
    assert(incompatible == nullptr);
}
