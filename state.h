#pragma once

#include <Arduino.h>
#include "config.h"

// Cross-task shared state. Guarded by gDataMux.
struct StateSnapshot {
  uint16_t raw[CH_COUNT];       // last CRSF channel values
  uint16_t outUs[CH_COUNT];     // last PPM pulse widths
  uint32_t lastRcMs;            // millis() of last valid RC frame, 0 = never
  uint32_t rcFrames;
  uint32_t crcErrors;
  uint32_t ppmFrames;
  uint32_t ppmErrors;
  uint16_t ppmPeriodUs;         // measured duration of last PPM frame
  bool ppmFailsafe;
  int16_t rssi;                 // dBm
  uint8_t lq;                   // %
  int8_t snr;                   // dB
  bool linkStatsValid;
};

extern portMUX_TYPE gDataMux;
extern StateSnapshot gState;

void stateInit();
StateSnapshot stateSnapshot();
void stateSetChannels(const uint16_t raw[CH_COUNT]);
void stateSetLinkStats(int16_t rssi, uint8_t lq, int8_t snr);
void stateCountCrcError();
void stateSetOutputs(const uint16_t out[CH_COUNT], uint16_t periodUs, bool failsafe, bool ok);
