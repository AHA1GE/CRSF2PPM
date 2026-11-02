#pragma once

#include <Arduino.h>

#define FW_VERSION "1.0.0"

#define CH_COUNT 16
#define CFG_MAGIC 0x4332504DUL  // 'C2PM'
#define CFG_VERSION 1

enum FailsafeMode : uint8_t {
  FS_HOLD = 0,
  FS_PRESET = 1,
};

// Persisted configuration. Keep this POD: it is stored as a binary blob in NVS.
struct AppConfig {
  uint32_t magic;
  uint16_t version;
  uint8_t count;              // number of PPM output slots, 1..16
  uint8_t map[CH_COUNT];      // output slot -> CRSF source channel (0-based)
  uint16_t failUs[CH_COUNT];  // failsafe pulse width per output slot
  uint16_t frameUs;           // nominal PPM frame period
  uint16_t pulseUs;           // PPM pulse width
  uint8_t invert;             // 0 = normal, 1 = inverted
  uint8_t failsafeMode;       // FailsafeMode
  uint16_t failsafeMs;        // link timeout before failsafe
  uint32_t crsfBaud;
  int8_t crsfPin;             // UART RX
  int8_t ppmPin;              // RMT TX
  char apPass[32];
  char staSsid[33];
  char staPass[65];
};

extern AppConfig gConfig;
extern portMUX_TYPE gCfgMux;

void configSetDefaults(AppConfig &c);
bool configLoad();
bool configSave();
bool configValidate(const AppConfig &c, String &err);
void configCommit(const AppConfig &c);
AppConfig configSnapshot();
bool configNeedsReboot(const AppConfig &a, const AppConfig &b);
