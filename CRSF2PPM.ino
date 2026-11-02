#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <ctype.h>
#include <esp_mac.h>

#include "app.h"
#include "config.h"
#include "crsf.h"
#include "ppm.h"
#include "state.h"
#include "webapp.h"

volatile bool gTestMode = false;
volatile uint16_t gCrsfFps = 0;
volatile uint16_t gPpmFps = 0;
char gApSsid[33] = "CRSF2PPM";

static bool s_rebootPending = false;
static uint32_t s_rebootAt = 0;
static uint32_t s_lastStatusMs = 0;
static uint32_t s_lastRateMs = 0;
static uint32_t s_lastStaTryMs = 0;
static uint32_t s_lastRcCount = 0;
static uint32_t s_lastPpmCount = 0;

static char s_cmd[64];
static uint8_t s_cmdLen = 0;

void appScheduleReboot(const char *reason, uint32_t delayMs) {
  if (s_rebootPending) {
    return;
  }
  s_rebootPending = true;
  s_rebootAt = millis() + delayMs;
  Serial.printf("[SYS] reboot in %lu ms (%s)\n", (unsigned long)delayMs, reason);
}

void appPrintHelp() {
  Serial.println("[HELP] s = status, t = toggle test mode, x = CRSF self-test,");
  Serial.println("[HELP] r = reboot, d = restore defaults + reboot, ? = help");
}

void appSerialStatus() {
  const AppConfig c = configSnapshot();
  const StateSnapshot s = stateSnapshot();
  const uint32_t now = millis();
  const uint32_t age = s.lastRcMs ? (now - s.lastRcMs) : 0xFFFFFFFFUL;
  const bool fresh = s.lastRcMs != 0 && age <= c.failsafeMs;
  const char *state = gTestMode ? "TEST" : (fresh ? "LINK" : (s.lastRcMs ? "FAILSAFE" : "NO_LINK"));

  Serial.printf(
      "[STATUS] up=%lus state=%s fps=%u/%u crc=%lu rssi=%d lq=%u snr=%d ppm=%lu err=%lu "
      "per=%uus fs=%u test=%u heap=%lu\n",
      (unsigned long)(now / 1000), state, (unsigned)gCrsfFps, (unsigned)gPpmFps,
      (unsigned long)s.crcErrors, (int)s.rssi, (unsigned)s.lq, (int)s.snr,
      (unsigned long)s.ppmFrames, (unsigned long)s.ppmErrors, (unsigned)s.ppmPeriodUs,
      s.ppmFailsafe ? 1U : 0U, gTestMode ? 1U : 0U, (unsigned long)ESP.getFreeHeap());
  Serial.print("[STATUS] out:");
  for (uint8_t i = 0; i < c.count; i++) {
    Serial.printf(" %u", s.outUs[i]);
  }
  Serial.println();
  Serial.printf("[WIFI] ap=%s ip=%s sta=%s%s%s rssi=%d\n", gApSsid,
                WiFi.softAPIP().toString().c_str(),
                c.staSsid[0] ? c.staSsid : "(off)",
                (WiFi.status() == WL_CONNECTED) ? " up " : " down ",
                (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString().c_str() : "",
                (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0);
}

static void handleCommand(const char *cmd) {
  const char c = (char)tolower((unsigned char)cmd[0]);
  switch (c) {
    case 's':
      appSerialStatus();
      break;
    case 't':
      gTestMode = !gTestMode;
      Serial.printf("[TEST] mode %s\n", gTestMode ? "ON" : "OFF");
      break;
    case 'x': {
      uint16_t decoded[CH_COUNT];
      bool crcRejected = false;
      const bool pass = crsfSelfTest(decoded, crcRejected);
      Serial.printf("[TEST] CRSF self-test %s (crc rejected: %s)\n", pass ? "PASS" : "FAIL",
                    crcRejected ? "yes" : "no");
      Serial.printf("[TEST] decoded:");
      for (uint8_t i = 0; i < CH_COUNT; i++) {
        Serial.printf(" %u", decoded[i]);
      }
      Serial.println();
      break;
    }
    case 'r':
      Serial.println("[SYS] reboot requested");
      appScheduleReboot("serial request", 200);
      break;
    case 'd': {
      AppConfig def;
      configSetDefaults(def);
      configCommit(def);
      Serial.println("[CFG] defaults restored");
      appScheduleReboot("defaults restored", 300);
      break;
    }
    case '?':
      appPrintHelp();
      break;
    default:
      Serial.printf("[HELP] unknown command '%c'. Type ? for help.\n", c);
      break;
  }
}

static void serialConsole() {
  while (Serial.available() > 0) {
    const int ci = Serial.read();
    if (ci < 0) {
      break;
    }
    const char c = (char)ci;
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      s_cmd[s_cmdLen] = '\0';
      if (s_cmdLen > 0) {
        handleCommand(s_cmd);
      }
      s_cmdLen = 0;
    } else if (s_cmdLen < sizeof(s_cmd) - 1) {
      s_cmd[s_cmdLen++] = c;
    }
  }
}

static void updateRates() {
  const uint32_t now = millis();
  const uint32_t elapsed = now - s_lastRateMs;
  if (elapsed < 1000) {
    return;
  }
  const StateSnapshot s = stateSnapshot();
  gCrsfFps = (uint16_t)((s.rcFrames - s_lastRcCount) * 1000UL / elapsed);
  gPpmFps = (uint16_t)((s.ppmFrames - s_lastPpmCount) * 1000UL / elapsed);
  s_lastRcCount = s.rcFrames;
  s_lastPpmCount = s.ppmFrames;
  s_lastRateMs = now;
}

static void wifiSetup(const AppConfig &cfg) {
  WiFi.persistent(false);
  WiFi.setHostname("crsf2ppm");

  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
  snprintf(gApSsid, sizeof(gApSsid), "CRSF2PPM-%02X%02X", mac[4], mac[5]);

  const bool useSta = cfg.staSsid[0] != '\0';
  WiFi.mode(useSta ? WIFI_AP_STA : WIFI_AP);
  WiFi.setSleep(false);  // keep the AP responsive while the Web UI is polled
  if (!WiFi.softAP(gApSsid, cfg.apPass)) {
    Serial.println("[WIFI] softAP start FAILED");
  }
  if (useSta) {
    WiFi.begin(cfg.staSsid, cfg.staPass);
    Serial.printf("[WIFI] STA connecting to \"%s\"\n", cfg.staSsid);
  }

  delay(150);
  Serial.printf("[WIFI] AP SSID \"%s\" (WPA2)  ->  http://%s/\n", gApSsid,
                WiFi.softAPIP().toString().c_str());
  if (MDNS.begin("crsf2ppm")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("[WIFI] mDNS http://crsf2ppm.local/");
  }
}

static void staMaintain(const AppConfig &cfg) {
  if (cfg.staSsid[0] == '\0') {
    return;
  }
  const wl_status_t st = WiFi.status();
  if (st == WL_CONNECTED || st == WL_IDLE_STATUS) {
    return;
  }
  const uint32_t now = millis();
  if (now - s_lastStaTryMs >= 30000) {
    s_lastStaTryMs = now;
    Serial.printf("[WIFI] STA retry \"%s\" (status %d)\n", cfg.staSsid, (int)st);
    WiFi.begin(cfg.staSsid, cfg.staPass);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1200);  // give USB CDC a moment to enumerate

  Serial.println();
  Serial.println("========================================");
  Serial.printf(" CRSF2PPM %s\n", FW_VERSION);
  Serial.printf(" Chip: %s rev %u, flash %lu MB\n", ESP.getChipModel(), ESP.getChipRevision(),
                (unsigned long)(ESP.getFlashChipSize() / (1024 * 1024)));
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
  Serial.printf(" MAC:  %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3],
                mac[4], mac[5]);
  Serial.println("========================================");

  stateInit();
  configLoad();
  AppConfig cfg = configSnapshot();

  if (ppmInit(cfg.ppmPin)) {
    ppmStart();
    Serial.printf("[PPM] RMT TX=GPIO%d, %uch, frame %uus, pulse %uus, %s\n", cfg.ppmPin,
                  cfg.count, cfg.frameUs, cfg.pulseUs, cfg.invert ? "inverted" : "normal");
  } else {
    Serial.println("[PPM] RMT init FAILED - check PPM pin");
  }

  crsfBegin(cfg);
  wifiSetup(cfg);
  webSetup();

  s_lastRateMs = millis();
  s_lastStatusMs = millis();
  s_lastStaTryMs = millis();
  s_lastRcCount = stateSnapshot().rcFrames;
  s_lastPpmCount = stateSnapshot().ppmFrames;

  Serial.println("----------------------------------------");
  Serial.println(" Web UI is ready. Wiring defaults:");
  Serial.printf("   CRSF receiver TX -> GPIO%d (ESP32-C3)\n", cfg.crsfPin);
  Serial.printf("   PPM out          -> GPIO%d -> flight controller\n", cfg.ppmPin);
  Serial.println("   Receiver 5V/GND and FC GND must share ground with the board.");
  Serial.println(" Serial: s=status t=test x=selftest r=reboot d=defaults ?=help");
  Serial.println("----------------------------------------");
  appSerialStatus();
}

void loop() {
  webLoop();
  serialConsole();
  updateRates();
  staMaintain(configSnapshot());

  if (s_rebootPending && (int32_t)(millis() - s_rebootAt) >= 0) {
    Serial.println("[SYS] restarting...");
    Serial.flush();
    delay(50);
    ESP.restart();
  }

  if (millis() - s_lastStatusMs >= 15000) {
    s_lastStatusMs = millis();
    appSerialStatus();
  }

  delay(2);
}
