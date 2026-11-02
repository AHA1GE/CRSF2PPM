#pragma once

#include <Arduino.h>

extern volatile bool gTestMode;
extern volatile uint16_t gCrsfFps;
extern volatile uint16_t gPpmFps;
extern char gApSsid[33];

void appScheduleReboot(const char *reason, uint32_t delayMs = 1500);
void appSerialStatus();
void appPrintHelp();

// Wi-Fi is runtime-only and starts OFF after every power-up.
bool appWifiEnabled();
void appToggleWifi();
void appRequestWifiToggle();
bool appTakeWifiToggleRequest();

// Latched fault: blinks the status LED at 5 Hz until reset.
void appSetError(const char *reason);
bool appHasError();
const char *appErrorReason();
