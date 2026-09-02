#include <Arduino.h>
#include <Strata.h>

void setup() {
    Serial.begin(115200);

    Serial.printf("DMA supported: %s\n", Strata::supports(Strata::Capability::Dma) ? "yes" : "no");
    Serial.printf("Executable supported: %s\n", Strata::supports(Strata::Capability::Executable) ? "yes" : "no");

    void *dma = Strata::allocate(Strata::AllocationRequest{
        .sizeBytes = 1024,
        .placement = Strata::Placement::Internal,
        .alignment = 32,
        .capabilities = Strata::Capability::Dma,
    });

    if (dma != nullptr) {
        Serial.printf("DMA allocation region: %u\n", static_cast<unsigned>(Strata::regionOf(dma)));
        Strata::free(dma);
    } else {
        Serial.println("DMA allocation unavailable");
    }
}

void loop() {}
