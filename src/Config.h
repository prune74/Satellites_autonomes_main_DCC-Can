#pragma once

// Bitrate CAN Booster Discovery (EXSA utilise 500 kbps)
static const uint32_t CAN_BOOSTER_BITRATE = 500000;

// Seuils DCC (µs)
static const uint32_t DCC_MIN_1 = 40;   // bit 1 ≈ 58 µs
static const uint32_t DCC_MAX_1 = 80;

static const uint32_t DCC_MIN_0 = 90;   // bit 0 ≈ 116 µs
static const uint32_t DCC_MAX_0 = 150;

// Taille de la file d’événements
static const uint8_t DCC_EVENT_QUEUE_SIZE = 32;
