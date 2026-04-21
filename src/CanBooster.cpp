#include "CanBooster.h"
#include "pins.h"
#include "config.h"

void CanBooster_begin() {
    ACAN_ESP32_Settings settings(CAN_BOOSTER_BITRATE);
    settings.mTxPin = PIN_CAN_TX;
    settings.mRxPin = PIN_CAN_RX;

    uint32_t err = ACAN_ESP32::can.begin(settings);
    if (err != 0) {
        Serial.printf("CAN error 0x%X\n", err);
    } else {
        Serial.println("CAN Booster OK");
    }
}

void CanBooster_sendDcc(const DccEvent &ev) {
    CANMessage msg;

    switch (ev.type) {
        case DCC_BIT_0:
            msg.id = 0x100;
            msg.len = 2;
            msg.data[0] = 0;
            msg.data[1] = ev.phase;
            break;

        case DCC_BIT_1:
            msg.id = 0x100;
            msg.len = 2;
            msg.data[0] = 1;
            msg.data[1] = ev.phase;
            break;

        case DCC_CUTOUT_START:
            msg.id = 0x101;
            msg.len = 2;
            msg.data[0] = 1;
            msg.data[1] = 1;
            break;

        case DCC_CUTOUT_END:
            msg.id = 0x101;
            msg.len = 2;
            msg.data[0] = 0;
            msg.data[1] = 0;
            break;
    }

    ACAN_ESP32::can.tryToSend(msg);
}
