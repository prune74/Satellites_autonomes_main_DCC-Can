#pragma once
#include <ACAN_ESP32.h>
#include "DccDecoder.h"

void CanBooster_begin();
void CanBooster_sendDcc(const DccEvent &ev);
