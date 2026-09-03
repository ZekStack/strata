#include <Arduino.h>
#include <strata/freertos/BinarySemaphore.h>

Strata::FreeRTOS::BinarySemaphore ready;

void setup() {
	Serial.begin(115200);

	ready = Strata::FreeRTOS::BinarySemaphore::create();
	if (!ready) {
		Serial.println("binary semaphore creation failed");
		return;
	}

	Serial.println("binary semaphore starts empty");
}

void loop() {
	if (!ready) {
		delay(1000);
		return;
	}

	if (ready.give()) {
		Serial.println("signaled");
	}

	if (ready.take(pdMS_TO_TICKS(100))) {
		Serial.println("signal consumed");
	}

	delay(1000);
}
