#include <Arduino.h>
#include "pins.h"
#include "config.h"
#include "DccDecoder.h"
#include "CanBooster.h"
#include "Cli.h"
#include "CanUniversal/CanInit.h"
#include "CanUniversal/CanMsg.h"
#include "CanUniversal/CanBus.h"

TaskHandle_t taskDccHandle = nullptr;
TaskHandle_t taskCanHandle = nullptr;
TaskHandle_t taskCanRxHandle = nullptr;

volatile bool canMonitorEnabled = false;
volatile int32_t canMonitorFilter = -1;

// ---------------------------------------------------------------------------
// Tâche DCC (core 0)
// ---------------------------------------------------------------------------
void taskDcc(void *pvParameters) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ---------------------------------------------------------------------------
// Tâche CAN (core 1)
// ---------------------------------------------------------------------------
void taskCan(void *pvParameters) {
    DccEvent ev;
    uint32_t lastDccMs = millis();
    uint32_t lastStatsMs = millis();

    bool failsafeActive = false;
    uint32_t failsafeSince = 0;

    for (;;) {

        // 1) Événement DCC disponible
        if (DccDecoder_getEvent(ev)) {
            lastDccMs = millis();

            if (failsafeActive) {
                failsafeActive = false;
                CanBooster_sendTelemetry(0, 0, BOOSTER_OK);
            }

            switch (ev.type) {
                case DCC_EVT_BIT:
                    CanBooster_sendDccBit(ev.bit, ev.phase);
                    break;

                case DCC_EVT_CUTOUT_START:
                    CanBooster_sendCutout(true, true);
                    break;

                case DCC_EVT_CUTOUT_END:
                    CanBooster_sendCutout(false, false);
                    break;
            }

            digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        }

        // 2) Aucun événement → FAILSAFE + stats
        else {
            uint32_t now = millis();

            if (!failsafeActive && (now - lastDccMs > DCCB_FAILSAFE_TIMEOUT_MS)) {
                failsafeActive = true;
                failsafeSince = now;

                CanBooster_sendCutout(true, true);
                CanBooster_sendTelemetry(0, 0, BOOSTER_OFF);
            }

            if (failsafeActive && (now - failsafeSince > DCCB_FAILSAFE_COOLDOWN_MS)) {
                if (now - lastDccMs < DCCB_FAILSAFE_TIMEOUT_MS) {
                    failsafeActive = false;
                    CanBooster_sendTelemetry(0, 0, BOOSTER_OK);
                }
            }

#if DCCB_MEASURE_STATS
            if (now - lastStatsMs > DCCB_TELEMETRY_PERIOD_MS) {
                uint32_t b0, b1, co, bad;
                DccDecoder_getStats(b0, b1, co, bad);

                uint16_t pseudoCurrent = (uint16_t)((b0 + b1) & 0xFFFF);
                uint16_t pseudoVoltage = (uint16_t)(co & 0xFFFF);
                BoosterState st = (bad > 0) ? BOOSTER_FAULT : BOOSTER_OK;

                CanBooster_sendTelemetry(pseudoCurrent, pseudoVoltage, st);

                lastStatsMs = now;
            }
#endif

            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

// ---------------------------------------------------------------------------
// Tâche CAN RX (sniffer)
// ---------------------------------------------------------------------------
void taskCanRx(void *pvParameters) {
    CanMsg msg;

    for (;;) {
        if (CAN[0].receive(msg)) {

            if (canMonitorEnabled) {
                if (canMonitorFilter == -1 || msg.id == (uint32_t)canMonitorFilter) {

                    Serial.printf("[CAN RX] ID=0x%03X LEN=%u DATA=",
                                  msg.id, msg.dlc);

                    for (uint8_t i = 0; i < msg.dlc; i++)
                        Serial.printf("%02X ", msg.data[i]);

                    Serial.println();
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// ---------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(200);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    Serial.println("\nDCC_CANBooster FreeRTOS - 2 cores");

    DccDecoder_begin();
    CanUniversal_begin();   // <-- remplace ACAN_ESP32::can.begin()
    Cli_begin();

    xTaskCreatePinnedToCore(taskDcc, "DCC", 4096, nullptr, 2, &taskDccHandle, 0);
    xTaskCreatePinnedToCore(taskCan, "CAN", 4096, nullptr, 3, &taskCanHandle, 1);
    xTaskCreatePinnedToCore(taskCanRx, "CAN_RX", 4096, nullptr, 1, &taskCanRxHandle, 1);
}

// ---------------------------------------------------------------------------
// LOOP
// ---------------------------------------------------------------------------
void loop() {
    Cli_task();
    vTaskDelay(pdMS_TO_TICKS(10));
}
