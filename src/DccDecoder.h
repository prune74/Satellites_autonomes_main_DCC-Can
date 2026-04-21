#pragma once
#include <Arduino.h>

enum DccEventType {
    DCC_BIT_0,
    DCC_BIT_1,
    DCC_CUTOUT_START,
    DCC_CUTOUT_END
};

struct DccEvent {
    DccEventType type;
    uint8_t phase;
};

void DccDecoder_begin();
bool DccDecoder_getEvent(DccEvent &ev);
