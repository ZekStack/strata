#include <Arduino.h>
#include <strata/freertos/Task.h>

Strata::FreeRTOS::Task workerTask;

void worker(void *) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
    Serial.begin(115200);

    workerTask = Strata::FreeRTOS::Task::create(worker, nullptr, {
        .name = "strata-demo",
        .stackBytes = 4096,
        .stackPlacement = Strata::Placement::PreferExternal,
        .priority = 1,
        .affinity = -1,
    });

    if (workerTask) {
        Serial.printf("stack bytes=%u region=%u hwm=%u\n",
            static_cast<unsigned>(workerTask.stackSizeBytes()),
            static_cast<unsigned>(workerTask.stackRegion()),
            static_cast<unsigned>(workerTask.stackHighWaterMarkBytes()));
    }
}

void loop() {
    delay(1000);
}
