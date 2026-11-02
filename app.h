#pragma once

#include <Arduino.h>

extern volatile bool gTestMode;
extern volatile uint16_t gCrsfFps;
extern volatile uint16_t gPpmFps;
extern char gApSsid[33];

void appScheduleReboot(const char *reason, uint32_t delayMs = 1500);
void appSerialStatus();
void appPrintHelp();
