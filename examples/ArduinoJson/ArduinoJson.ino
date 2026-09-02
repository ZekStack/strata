#include <Arduino.h>
#include <ArduinoJson.h>
#include <strata/arduinojson/Allocator.h>

void setup() {
	Serial.begin(115200);

	Strata::ArduinoJson::Allocator allocator{Strata::Placement::PreferExternal};
	ArduinoJson::JsonDocument document{&allocator};

	document["library"] = "Strata";
	document["placement"] = "prefer-external";
	auto values = document["values"].to<ArduinoJson::JsonArray>();
	for (int i = 0; i < 256; ++i) {
		values.add(i);
	}

	if (document.overflowed()) {
		Serial.println("ArduinoJson allocation failed");
		return;
	}

	serializeJson(document, Serial);
	Serial.println();
}

void loop() {
	delay(1000);
}
