#include "CanBooster.h"
#include "Config.h"
#include "CanUniversal/CanInit.h"
#include "CanUniversal/CanMsg.h"
#include "CanUniversal/CanBus.h"

// -----------------------------------------------------------------------------
// Envoi du bit DCC
// -----------------------------------------------------------------------------
bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase) {
    CanMsg msg((uint16_t)DCCB_CAN_ID_DCC_BIT, { bit, phase });
    return CAN[0].send(msg);
}

// -----------------------------------------------------------------------------
// Envoi du cutout
// -----------------------------------------------------------------------------
bool CanBooster_sendCutout(bool local, bool global) {
    CanMsg msg((uint16_t)DCCB_CAN_ID_CUTOUT,
               { uint8_t(local), uint8_t(global) });
    return CAN[0].send(msg);
}

// -----------------------------------------------------------------------------
// Envoi de la télémétrie
// -----------------------------------------------------------------------------
bool CanBooster_sendTelemetry(uint16_t mA, uint16_t mV, BoosterState st) {
    CanMsg msg((uint16_t)DCCB_CAN_ID_TELEMETRY,
               { uint8_t(mA / 10), uint8_t(mV / 100), uint8_t(st) });
    return CAN[0].send(msg);
}

// -----------------------------------------------------------------------------
// Envoi de l’adresse RailCom
// -----------------------------------------------------------------------------
bool CanBooster_sendRailcomAddress(uint16_t addr) {
    CanMsg msg((uint16_t)DCCB_CAN_ID_RAILCOM,
               { uint8_t(addr & 0xFF), uint8_t(addr >> 8) });
    return CAN[0].send(msg);
}
