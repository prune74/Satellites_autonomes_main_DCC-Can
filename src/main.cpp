#include <Arduino.h>
#include "pins.h"
#include "DccDecoder.h"
#include "CanBooster.h"

void setup() {
    Serial.begin(115200);
    delay(200);

    pinMode(PIN_LED, OUTPUT);

    Serial.println("\nDCC_CANBooster - ESP32 DevKitC");

    DccDecoder_begin();
    CanBooster_begin();
}

void loop() {
    DccEvent ev;

    while (DccDecoder_getEvent(ev)) {
        CanBooster_sendDcc(ev);
        digitalWrite(PIN_LED, !digitalRead(PIN_LED)); // debug
    }
}
