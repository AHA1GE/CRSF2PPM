#pragma once

#include <Arduino.h>
#include "config.h"

void crsfBegin(const AppConfig &cfg);
void crsfFeedByte(uint8_t b);
void crsfResetParser();
bool crsfSelfTest(uint16_t decoded[CH_COUNT], bool &crcRejected);
