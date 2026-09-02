#include <Strata.h>

void printStats(const char *name, Strata::Region region) {
    Serial.printf("%s supported: %s\n", name, Strata::supports(region) ? "yes" : "no");

    const auto stats = Strata::memoryStats(region);
    if (!stats.freeBytes.has_value()) {
        Serial.println("  statistics unavailable");
        return;
    }

    Serial.printf("  total: %u bytes\n", static_cast<unsigned>(*stats.totalBytes));
    Serial.printf("  free: %u bytes\n", static_cast<unsigned>(*stats.freeBytes));
    Serial.printf("  minimum free: %u bytes\n", static_cast<unsigned>(*stats.minimumFreeBytes));
    Serial.printf("  largest block: %u bytes\n", static_cast<unsigned>(*stats.largestFreeBlockBytes));
}

void setup() {
    Serial.begin(115200);

    void *ptr = Strata::allocate(1024, Strata::Placement::PreferExternal);
    Serial.printf("allocation region: %s\n", Strata::toString(Strata::regionOf(ptr)));

    printStats("internal", Strata::Region::Internal);
    printStats("external", Strata::Region::External);

    Strata::free(ptr);
}

void loop() {}
