#include <Arduino.h>
#include <strata/freertos/Queue.h>

Strata::FreeRTOS::Queue<int> queue;

void setup() {
	Serial.begin(115200);

	queue = Strata::FreeRTOS::Queue<int>::create({
		.length = 8,
		.storagePlacement = Strata::Placement::PreferExternal,
		.usage = Strata::FreeRTOS::QueueUsage::TaskOnly,
	});
	if (!queue) {
		Serial.println("queue allocation failed");
		return;
	}

	Serial.printf(
		"queue storage region=%u bytes=%u\n",
		static_cast<unsigned>(queue.storageRegion()),
		static_cast<unsigned>(queue.storageBytes()));

	int value = 42;
	if (queue.send(value)) {
		int received = 0;
		if (queue.receive(received)) {
			Serial.printf("received=%d\n", received);
		}
	}
}

void loop() {
	delay(1000);
}
