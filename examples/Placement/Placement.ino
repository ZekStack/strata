#include <Arduino.h>
#include <Strata.h>

void setup() {
    Serial.begin(115200);

    const Strata::Placement placement = Strata::Placement::PreferExternal;
    const Strata::Region region = Strata::Region::Unknown;

    Serial.printf("requested placement: %s\n", Strata::toString(placement));
    Serial.printf("actual region: %s\n", Strata::toString(region));
}

void loop() {
    delay(1000);
}
