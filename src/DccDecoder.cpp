#include "DccDecoder.h"
#include "Pins.h"
#include "Config.h"

static volatile uint32_t lastEdge = 0;
static volatile uint8_t currentPhase = 0;

// File d’événements
static DccEvent queue[DCC_EVENT_QUEUE_SIZE];
static volatile uint8_t head = 0, tail = 0;

static void IRAM_ATTR pushEvent(DccEventType t)
{
    uint8_t next = (head + 1) % DCC_EVENT_QUEUE_SIZE;
    if (next != tail)
    {
        queue[head].type = t;
        queue[head].phase = currentPhase;
        head = next;
    }
}

static void IRAM_ATTR dccISR()
{
    uint32_t now = micros();
    uint32_t dt = now - lastEdge;
    lastEdge = now;

    // Détection cutout (pas de front pendant longtemps)
    if (dt > 300)
    {
        pushEvent(DCC_CUTOUT_START);
        return;
    }

    // Classification bit 1
    if (dt >= DCC_MIN_1 && dt <= DCC_MAX_1)
    {
        currentPhase ^= 1;
        pushEvent(DCC_BIT_1);
        return;
    }

    // Classification bit 0
    if (dt >= DCC_MIN_0 && dt <= DCC_MAX_0)
    {
        currentPhase ^= 1;
        pushEvent(DCC_BIT_0);
        return;
    }
}

void DccDecoder_begin()
{
    pinMode(PIN_DCC_IN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_DCC_IN), dccISR, CHANGE);
}

bool DccDecoder_getEvent(DccEvent &ev)
{
    if (tail == head)
        return false;
    ev = queue[tail];
    tail = (tail + 1) % DCC_EVENT_QUEUE_SIZE;
    return true;
}
