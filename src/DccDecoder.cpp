#include "DccDecoder.h"
#include "pins.h"
#include "config.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

static volatile uint32_t lastEdge = 0;
static volatile uint8_t currentPhase = 0;

static QueueHandle_t dccQueue = nullptr;

static void IRAM_ATTR dccISR() {
    uint32_t now = micros();
    uint32_t dt = now - lastEdge;
    lastEdge = now;

    DccEvent ev;

    if (dt > 300) {
        ev.type = DCC_CUTOUT_START;
        ev.phase = currentPhase;
        xQueueSendFromISR(dccQueue, &ev, nullptr);
        return;
    }

    if (dt >= DCC_MIN_1 && dt <= DCC_MAX_1) {
        currentPhase ^= 1;
        ev.type = DCC_BIT_1;
        ev.phase = currentPhase;
        xQueueSendFromISR(dccQueue, &ev, nullptr);
        return;
    }

    if (dt >= DCC_MIN_0 && dt <= DCC_MAX_0) {
        currentPhase ^= 1;
        ev.type = DCC_BIT_0;
        ev.phase = currentPhase;
        xQueueSendFromISR(dccQueue, &ev, nullptr);
        return;
    }
}

void DccDecoder_begin() {
    dccQueue = xQueueCreate(DCC_EVENT_QUEUE_SIZE, sizeof(DccEvent));

    pinMode(PIN_DCC_IN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_DCC_IN), dccISR, CHANGE);
}

bool DccDecoder_getEvent(DccEvent &ev) {
    if (dccQueue == nullptr) return false;
    return xQueueReceive(dccQueue, &ev, 0) == pdTRUE;
}
