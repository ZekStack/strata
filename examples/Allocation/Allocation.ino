#include <Arduino.h>
#include <Strata.h>

void setup() {
    void *internal = Strata::allocate(256, Strata::Placement::Internal);
    void *bulk = Strata::allocate(4096, Strata::Placement::PreferExternal);

    auto *zeroed = Strata::calloc(32, sizeof(std::uint32_t), Strata::Placement::PreferExternal);

    void *aligned = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 512,
        .placement = Strata::Placement::PreferExternal,
        .alignment = 64,
    });

    Strata::free(aligned);
    Strata::free(zeroed);
    Strata::free(bulk);
    Strata::free(internal);
}

void loop() {}
