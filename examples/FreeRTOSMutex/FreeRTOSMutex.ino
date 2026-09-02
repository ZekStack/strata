#include <Arduino.h>
#include <strata/freertos/Mutex.h>

Strata::FreeRTOS::Mutex mutex;
Strata::FreeRTOS::RecursiveMutex recursiveMutex;

void setup() {
	Serial.begin(115200);

	mutex = Strata::FreeRTOS::Mutex::create();
	recursiveMutex = Strata::FreeRTOS::RecursiveMutex::create();
	if (!mutex || !recursiveMutex) {
		Serial.println("mutex allocation failed");
		return;
	}

	Serial.printf(
		"mutex region=%u recursive region=%u\n",
		static_cast<unsigned>(mutex.controlRegion()),
		static_cast<unsigned>(recursiveMutex.controlRegion()));

	if (mutex.tryLock()) {
		Serial.println("mutex locked");
		mutex.unlock();
	}

	if (recursiveMutex.lock()) {
		if (recursiveMutex.tryLock()) {
			recursiveMutex.unlock();
		}
		recursiveMutex.unlock();
	}
}

void loop() {
	delay(1000);
}
