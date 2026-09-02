#include <Strata.h>

#include <cstdint>

void setup() {
    Serial.begin(115200);

    Strata::Buffer buffer(1024, Strata::Placement::PreferExternal);
    if (buffer.empty()) {
        Serial.println("Buffer allocation failed");
        return;
    }

    buffer.data<std::uint8_t>()[0] = 42;

    Serial.printf("size=%u region=%s\n",
        static_cast<unsigned>(buffer.size()),
        Strata::toString(buffer.region()));

    if (buffer.resize(4096)) {
        Serial.printf("resized=%u first=%u\n",
            static_cast<unsigned>(buffer.size()),
            static_cast<unsigned>(buffer.data<std::uint8_t>()[0]));
    }

    void *raw = buffer.release();
    Strata::free(raw);
}

void loop() {}
