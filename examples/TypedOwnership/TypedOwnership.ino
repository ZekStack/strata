#include <Arduino.h>
#include <Strata.h>

struct Sample {
    explicit Sample(int initial) noexcept : value(initial) {}
    ~Sample() noexcept = default;

    int value;
};

void setup() {
    Serial.begin(115200);

    auto *samples = Strata::allocateArray<Sample>(8, Strata::Placement::PreferExternal);
    Serial.printf("raw typed storage: %s\n", samples != nullptr ? "ok" : "unavailable");
    Strata::free(samples);

    auto owned = Strata::makeUnique<Sample>(Strata::Placement::PreferExternal, 42);
    if (owned) {
        Serial.printf("owned value: %d\n", owned->value);
    }

    auto *manual = Strata::create<Sample>(Strata::Placement::Internal, 7);
    if (manual != nullptr) {
        Serial.printf("manual value: %d\n", manual->value);
        Strata::destroy(manual);
    }
}

void loop() {}
