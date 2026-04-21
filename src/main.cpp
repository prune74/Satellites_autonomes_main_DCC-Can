#include <Arduino.h>
#include "pins.h"
#include "DccDecoder.h"
#include "CanBooster.h"

TaskHandle_t taskDccHandle = nullptr;
TaskHandle_t taskCanHandle = nullptr;

// Tâche DCC (core 0) : ne fait rien d’autre que laisser l’ISR tourner
void taskDcc(void *pvParameters) {
    for (;;) {
        // On pourrait ajouter du debug timing ici si besoin
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Tâche CAN (core 1) : lit la queue et envoie les trames
void taskCan(void *pvParameters) {
    DccEvent ev;

    for (;;) {
        if (DccDecoder_getEvent(ev)) {
            CanBooster_sendDcc(ev);
            digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(200);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    Serial.println("\nDCC_CANBooster FreeRTOS - 2 cores");

    DccDecoder_begin();
    CanBooster_begin();

    // Tâche DCC sur core 0
    xTaskCreatePinnedToCore(
        taskDcc,
        "DCC",
        4096,
        nullptr,
        2,
        &taskDccHandle,
        0
    );

    // Tâche CAN sur core 1
    xTaskCreatePinnedToCore(
        taskCan,
        "CAN",
        4096,
        nullptr,
        3,
        &taskCanHandle,
        1
    );
}

void loop() {
    // Ne rien faire : tout est dans les tâches
    vTaskDelay(pdMS_TO_TICKS(1000));
}
