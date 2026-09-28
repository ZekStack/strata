#include <Arduino.h>
#include <strata/freertos/CountingSemaphore.h>

Strata::FreeRTOS::CountingSemaphore slots;

void setup() {
	Serial.begin(115200);

	slots = Strata::FreeRTOS::CountingSemaphore::create(3, 3);
	if (!slots) {
		Serial.println("counting semaphore creation failed");
		return;
	}

	Serial.printf("available slots start at %u\n", static_cast<unsigned>(slots.maxCount()));
}

void loop() {
	if (!slots) {
		delay(1000);
		return;
	}

	if (slots.tryTake()) {
		Serial.println("slot acquired");
		slots.give();
		Serial.println("slot released");
	}

	delay(1000);
}
