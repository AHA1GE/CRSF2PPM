#include "config.h"

#include <Preferences.h>
#include <cstring>

AppConfig gConfig;
portMUX_TYPE gCfgMux = portMUX_INITIALIZER_UNLOCKED;

static bool validPin(int p) {
  // ESP32-C3: GPIO0..10 are general purpose, 11..17 are SPI flash,
  // 18/19 are USB, 20/21 are UART0. Allow the general pins plus UART0.
  return (p >= 0 && p <= 10) || p == 20 || p == 21;
}

static void copyStr(char *dst, size_t n, const char *src) {
  if (src == nullptr) {
    src = "";
  }
  strlcpy(dst, src, n);
}

static bool validBaud(uint32_t b) {
  return b == 420000 || b == 400000 || b == 115200;
}

void configSetDefaults(AppConfig &c) {
  memset(&c, 0, sizeof(c));
  c.magic = CFG_MAGIC;
  c.version = CFG_VERSION;
  c.count = 8;
  for (uint8_t i = 0; i < CH_COUNT; i++) {
    c.map[i] = i;
    c.failUs[i] = 1500;
  }
  // AETR: output slot 3 is normally throttle. Preset it low on failsafe.
  c.failUs[2] = 1000;
  c.frameUs = 22500;
  c.pulseUs = 400;
  c.invert = 0;
  c.failsafeMode = FS_PRESET;
  c.failsafeMs = 500;
  c.crsfBaud = 420000;
  c.crsfPin = 3;
  c.ppmPin = 4;
  copyStr(c.apPass, sizeof(c.apPass), "crsf2ppm");
}

bool configValidate(const AppConfig &c, String &err) {
  if (c.count < 1 || c.count > CH_COUNT) {
    err = "PPM output count must be 1-16";
    return false;
  }
  for (uint8_t i = 0; i < CH_COUNT; i++) {
    if (c.map[i] >= CH_COUNT) {
      err = "Channel mapping out of range";
      return false;
    }
    if (c.failUs[i] < 800 || c.failUs[i] > 2200) {
      err = "Failsafe values must be 800-2200 us";
      return false;
    }
  }
  if (c.frameUs < 15000 || c.frameUs > 30000) {
    err = "Frame period must be 15000-30000 us";
    return false;
  }
  if (c.pulseUs < 200 || c.pulseUs > 700) {
    err = "Pulse width must be 200-700 us";
    return false;
  }
  if (c.failsafeMs < 100 || c.failsafeMs > 5000) {
    err = "Failsafe timeout must be 100-5000 ms";
    return false;
  }
  if (!validBaud(c.crsfBaud)) {
    err = "Unsupported CRSF baud rate";
    return false;
  }
  if (!validPin(c.crsfPin) || !validPin(c.ppmPin)) {
    err = "Invalid GPIO for this chip";
    return false;
  }
  if (c.crsfPin == c.ppmPin) {
    err = "CRSF and PPM pins must differ";
    return false;
  }

  size_t apLen = strnlen(c.apPass, sizeof(c.apPass));
  if (apLen < 8 || apLen >= sizeof(c.apPass)) {
    err = "AP password must be 8-31 characters";
    return false;
  }
  size_t ssidLen = strnlen(c.staSsid, sizeof(c.staSsid));
  if (ssidLen >= sizeof(c.staSsid)) {
    err = "STA SSID is too long";
    return false;
  }
  size_t passLen = strnlen(c.staPass, sizeof(c.staPass));
  if (passLen >= sizeof(c.staPass)) {
    err = "STA password is too long";
    return false;
  }
  if (ssidLen > 0 && (passLen < 8 || passLen >= sizeof(c.staPass))) {
    err = "STA password must be 8-64 characters";
    return false;
  }
  return true;
}

bool configLoad() {
  AppConfig c;
  configSetDefaults(c);

  Preferences prefs;
  bool ok = false;
  if (prefs.begin("crsf2ppm", true)) {
    size_t n = prefs.getBytesLength("cfg");
    if (n == sizeof(c) && prefs.getBytes("cfg", &c, sizeof(c)) == sizeof(c)) {
      ok = true;
    }
    prefs.end();
  }

  c.apPass[sizeof(c.apPass) - 1] = '\0';
  c.staSsid[sizeof(c.staSsid) - 1] = '\0';
  c.staPass[sizeof(c.staPass) - 1] = '\0';

  String err;
  if (!ok || c.magic != CFG_MAGIC || c.version != CFG_VERSION || !configValidate(c, err)) {
    if (ok) {
      Serial.printf("[CFG] stored config invalid (%s), restoring defaults\n", err.c_str());
    } else {
      Serial.println("[CFG] no stored config, using defaults");
    }
    configSetDefaults(c);
    portENTER_CRITICAL(&gCfgMux);
    gConfig = c;
    portEXIT_CRITICAL(&gCfgMux);
    configSave();
    return false;
  }

  portENTER_CRITICAL(&gCfgMux);
  gConfig = c;
  portEXIT_CRITICAL(&gCfgMux);
  Serial.println("[CFG] loaded from NVS");
  return true;
}

bool configSave() {
  AppConfig c = configSnapshot();
  Preferences prefs;
  if (!prefs.begin("crsf2ppm", false)) {
    Serial.println("[CFG] NVS open failed");
    return false;
  }
  size_t written = prefs.putBytes("cfg", &c, sizeof(c));
  prefs.end();
  if (written != sizeof(c)) {
    Serial.println("[CFG] NVS write failed");
    return false;
  }
  return true;
}

void configCommit(const AppConfig &c) {
  portENTER_CRITICAL(&gCfgMux);
  gConfig = c;
  portEXIT_CRITICAL(&gCfgMux);
  configSave();
}

AppConfig configSnapshot() {
  AppConfig c;
  portENTER_CRITICAL(&gCfgMux);
  c = gConfig;
  portEXIT_CRITICAL(&gCfgMux);
  return c;
}

bool configNeedsReboot(const AppConfig &a, const AppConfig &b) {
  return a.crsfBaud != b.crsfBaud || a.crsfPin != b.crsfPin || a.ppmPin != b.ppmPin ||
         strcmp(a.apPass, b.apPass) != 0 || strcmp(a.staSsid, b.staSsid) != 0 ||
         strcmp(a.staPass, b.staPass) != 0;
}
