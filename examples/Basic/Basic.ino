#include <Arduino.h>
#include <Strata.h>

void setup() {
	Serial.begin(115200);

	void *memory = Strata::allocate(4096, Strata::Placement::PreferExternal);
	if (memory == nullptr) {
		Serial.println("allocation failed");
		return;
	}

	Serial.printf("region=%u\n", static_cast<unsigned>(Strata::regionOf(memory)));
	Strata::free(memory);
}

void loop() {
	delay(1000);
}
