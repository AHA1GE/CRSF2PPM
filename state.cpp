#include "state.h"

#include <cstring>

StateSnapshot gState;
portMUX_TYPE gDataMux = portMUX_INITIALIZER_UNLOCKED;

void stateInit() {
  portENTER_CRITICAL(&gDataMux);
  memset(&gState, 0, sizeof(gState));
  portEXIT_CRITICAL(&gDataMux);
}

StateSnapshot stateSnapshot() {
  StateSnapshot s;
  portENTER_CRITICAL(&gDataMux);
  s = gState;
  portEXIT_CRITICAL(&gDataMux);
  return s;
}

void stateSetChannels(const uint16_t raw[CH_COUNT]) {
  portENTER_CRITICAL(&gDataMux);
  memcpy(gState.raw, raw, sizeof(gState.raw));
  gState.lastRcMs = millis();
  gState.rcFrames++;
  portEXIT_CRITICAL(&gDataMux);
}

void stateSetLinkStats(int16_t rssi, uint8_t lq, int8_t snr) {
  portENTER_CRITICAL(&gDataMux);
  gState.rssi = rssi;
  gState.lq = lq;
  gState.snr = snr;
  gState.linkStatsValid = true;
  portEXIT_CRITICAL(&gDataMux);
}

void stateCountCrcError() {
  portENTER_CRITICAL(&gDataMux);
  gState.crcErrors++;
  portEXIT_CRITICAL(&gDataMux);
}

void stateSetOutputs(const uint16_t out[CH_COUNT], uint16_t periodUs, bool failsafe, bool ok) {
  portENTER_CRITICAL(&gDataMux);
  memcpy(gState.outUs, out, sizeof(gState.outUs));
  gState.ppmPeriodUs = periodUs;
  gState.ppmFailsafe = failsafe;
  if (ok) {
    gState.ppmFrames++;
  } else {
    gState.ppmErrors++;
  }
  portEXIT_CRITICAL(&gDataMux);
}
