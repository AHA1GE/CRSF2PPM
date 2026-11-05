#include "webapp.h"

#include <Arduino.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <cstring>

#include "app.h"
#include "config.h"
#include "crsf.h"
#include "led.h"
#include "state.h"
#include "webui.h"

static WebServer server(80);
static DNSServer s_dns;
static bool s_webStarted = false;

static String jesc(const char *s) {
  String out;
  if (s == nullptr) {
    return out;
  }
  out.reserve(strlen(s) + 8);
  for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
    const uint8_t c = *p;
    if (c == '"' || c == '\\') {
      out += '\\';
      out += (char)c;
    } else if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else if (c == '\t') {
      out += "\\t";
    } else if (c < 0x20) {
      char buf[8];
      snprintf(buf, sizeof(buf), "\\u%04x", c);
      out += buf;
    } else {
      out += (char)c;
    }
  }
  return out;
}

// The bundled UI sends X-Lang with every request; plain API clients can use
// Accept-Language instead. Only the first language tag is considered.
static bool wantsChinese() {
  String lang = server.header("X-Lang");
  if (lang.length() == 0) {
    lang = server.header("Accept-Language");
  }
  lang.trim();
  lang.toLowerCase();
  return lang.startsWith("zh");
}

struct ErrorTranslation {
  const char *en;
  const char *zh;
};

static const ErrorTranslation kErrorTranslations[] = {
    {"not found", "未找到"},
    {"PPM output count must be 1-16", "PPM 输出数量必须为 1-16"},
    {"Channel mapping out of range", "通道映射超出范围"},
    {"Per-channel reverse must be 0 or 1", "每通道反向值必须为 0 或 1"},
    {"Failsafe values must be 800-2200 us", "失控保护值必须在 800-2200 µs 之间"},
    {"Frame period must be 15000-30000 us", "帧周期必须在 15000-30000 µs 之间"},
    {"Pulse width must be 200-700 us", "脉冲宽度必须在 200-700 µs 之间"},
    {"Failsafe timeout must be 100-5000 ms", "失控保护超时必须在 100-5000 ms 之间"},
    {"Unsupported CRSF baud rate", "不支持该 CRSF 波特率"},
    {"Invalid GPIO for this chip", "该 GPIO 对此芯片无效"},
    {"CRSF and PPM pins must differ", "CRSF 与 PPM 引脚不能相同"},
    {"GPIO8/GPIO9 are reserved for the status LED and Wi-Fi button",
     "GPIO8/GPIO9 已保留给状态 LED 和 Wi-Fi 按键"},
    {"STA SSID is too long", "STA SSID 过长"},
    {"STA password is too long", "STA 密码过长"},
    {"STA password must be 8-64 characters", "STA 密码长度必须为 8-64 个字符"},
    {"STA password required for the configured SSID", "已设置 STA SSID，必须填写 STA 密码"},
};

static String localizeMessage(const String &message) {
  if (!wantsChinese()) {
    return message;
  }
  for (const ErrorTranslation &t : kErrorTranslations) {
    if (message == t.en) {
      return String(t.zh);
    }
  }
  return message;
}

static String portalUrl() {
  String url = "http://";
  url += WiFi.softAPIP().toString();
  url += "/";
  return url;
}

static void sendJson(int code, const String &body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json", body);
}

static void sendError(int code, const String &message) {
  const String text = localizeMessage(message);
  sendJson(code, String("{\"ok\":false,\"error\":\"") + jesc(text.c_str()) + "\"}");
}

static void appendU16Array(String &j, const uint16_t *v, uint8_t n, bool oneBased = false) {
  j += '[';
  for (uint8_t i = 0; i < n; i++) {
    if (i) {
      j += ',';
    }
    j += String(oneBased ? (unsigned)(v[i] + 1) : (unsigned)v[i]);
  }
  j += ']';
}

static void appendMapArray(String &j, const uint8_t *v, uint8_t n) {
  j += '[';
  for (uint8_t i = 0; i < n; i++) {
    if (i) {
      j += ',';
    }
    j += String((unsigned)(v[i] + 1));
  }
  j += ']';
}

static void appendFlagArray(String &j, const uint8_t *v, uint8_t n) {
  j += '[';
  for (uint8_t i = 0; i < n; i++) {
    if (i) {
      j += ',';
    }
    j += v[i] ? '1' : '0';
  }
  j += ']';
}

static void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Captive-Portal", portalUrl());
  server.send_P(200, "text/html", INDEX_HTML);
}

static void handleStatus() {
  const AppConfig cfg = configSnapshot();
  const StateSnapshot st = stateSnapshot();
  const uint32_t now = millis();
  const uint32_t age = st.lastRcMs ? (now - st.lastRcMs) : 0xFFFFFFFFUL;
  const bool linkOk = st.lastRcMs != 0 && age <= cfg.failsafeMs;
  const bool staUp = WiFi.status() == WL_CONNECTED;

  String j;
  j.reserve(1400);
  j += "{\"version\":\"" FW_VERSION "\"";
  j += ",\"uptime_ms\":" + String((unsigned long)now);
  j += ",\"wifi\":{\"enabled\":";
  j += appWifiEnabled() ? "true" : "false";
  j += ",\"mode\":\"";
  j += (WiFi.getMode() == WIFI_AP_STA) ? "AP+STA" : "AP";
  j += "\",\"ap_ssid\":\"" + jesc(gApSsid) + "\"";
  j += ",\"ap_ip\":\"" + WiFi.softAPIP().toString() + "\"";
  j += ",\"sta_ssid\":\"" + jesc(cfg.staSsid) + "\"";
  j += ",\"sta_connected\":";
  j += staUp ? "true" : "false";
  j += ",\"sta_ip\":\"" + WiFi.localIP().toString() + "\"";
  j += ",\"sta_rssi\":" + String(staUp ? WiFi.RSSI() : 0) + "}";

  j += ",\"crsf\":{\"baud\":" + String((unsigned long)cfg.crsfBaud);
  j += ",\"frames\":" + String((unsigned long)st.rcFrames);
  j += ",\"crc_errors\":" + String((unsigned long)st.crcErrors);
  j += ",\"fps\":" + String((unsigned)gCrsfFps);
  j += ",\"age_ms\":" + String((unsigned long)age);
  j += ",\"rssi\":" + String(st.rssi);
  j += ",\"lq\":" + String(st.lq);
  j += ",\"snr\":" + String(st.snr);
  j += ",\"stats_valid\":";
  j += st.linkStatsValid ? "true" : "false";
  j += ",\"link_ok\":";
  j += linkOk ? "true" : "false";
  j += "}";

  j += ",\"ppm\":{\"frames\":" + String((unsigned long)st.ppmFrames);
  j += ",\"fps\":" + String((unsigned)gPpmFps);
  j += ",\"period_us\":" + String(st.ppmPeriodUs);
  j += ",\"errors\":" + String((unsigned long)st.ppmErrors);
  j += ",\"failsafe\":";
  j += st.ppmFailsafe ? "true" : "false";
  j += ",\"count\":" + String(cfg.count) + "}";

  j += ",\"test\":";
  j += gTestMode ? "true" : "false";
  j += ",\"led\":\"" + String(ledModeName()) + "\"";
  j += ",\"error\":";
  j += appHasError() ? "true" : "false";
  j += ",\"error_reason\":\"" + jesc(appErrorReason()) + "\"";
  j += ",\"heap\":" + String(ESP.getFreeHeap());
  j += ",\"raw\":";
  appendU16Array(j, st.raw, CH_COUNT);
  j += ",\"out\":";
  appendU16Array(j, st.outUs, CH_COUNT);
  j += '}';
  sendJson(200, j);
}

static void handleConfigGet() {
  const AppConfig c = configSnapshot();
  String j;
  j.reserve(900);
  j += "{\"ok\":true,\"version\":\"" FW_VERSION "\"";
  j += ",\"count\":" + String(c.count);
  j += ",\"map\":";
  appendMapArray(j, c.map, CH_COUNT);
  j += ",\"rev\":";
  appendFlagArray(j, c.rev, CH_COUNT);
  j += ",\"fs\":";
  appendU16Array(j, c.failUs, CH_COUNT);
  j += ",\"frame_us\":" + String(c.frameUs);
  j += ",\"pulse_us\":" + String(c.pulseUs);
  j += ",\"invert\":" + String(c.invert);
  j += ",\"failsafe_mode\":\"";
  j += (c.failsafeMode == FS_HOLD) ? "hold" : "preset";
  j += "\",\"failsafe_ms\":" + String(c.failsafeMs);
  j += ",\"crsf_baud\":" + String((unsigned long)c.crsfBaud);
  j += ",\"crsf_pin\":" + String((int)c.crsfPin);
  j += ",\"ppm_pin\":" + String((int)c.ppmPin);
  j += ",\"ap_ssid\":\"" + jesc(gApSsid) + "\"";
  j += ",\"ap_open\":true";
  j += ",\"sta_ssid\":\"" + jesc(c.staSsid) + "\"";
  j += ",\"sta_pass_set\":";
  j += (strlen(c.staPass) > 0) ? "true" : "false";
  j += '}';
  sendJson(200, j);
}

static void handleConfigPost() {
  const AppConfig old = configSnapshot();
  AppConfig next = old;

  if (server.hasArg("count")) {
    next.count = (uint8_t)server.arg("count").toInt();
  }
  for (uint8_t i = 0; i < CH_COUNT; i++) {
    String key = String("map") + i;
    if (server.hasArg(key)) {
      const int src = server.arg(key).toInt();
      next.map[i] = (uint8_t)(src - 1);
    }
    key = String("rev") + i;
    if (server.hasArg(key)) {
      const String v = server.arg(key);
      next.rev[i] = (v == "1" || v == "true" || v == "on") ? 1 : 0;
    }
    key = String("fs") + i;
    if (server.hasArg(key)) {
      next.failUs[i] = (uint16_t)server.arg(key).toInt();
    }
  }
  if (server.hasArg("frame_us")) {
    next.frameUs = (uint16_t)server.arg("frame_us").toInt();
  }
  if (server.hasArg("pulse_us")) {
    next.pulseUs = (uint16_t)server.arg("pulse_us").toInt();
  }
  if (server.hasArg("invert")) {
    next.invert = server.arg("invert").toInt() ? 1 : 0;
  }
  if (server.hasArg("failsafe_mode")) {
    next.failsafeMode = (server.arg("failsafe_mode") == "hold") ? FS_HOLD : FS_PRESET;
  }
  if (server.hasArg("failsafe_ms")) {
    next.failsafeMs = (uint16_t)server.arg("failsafe_ms").toInt();
  }
  if (server.hasArg("crsf_baud")) {
    next.crsfBaud = (uint32_t)server.arg("crsf_baud").toInt();
  }
  if (server.hasArg("crsf_pin")) {
    next.crsfPin = (int8_t)server.arg("crsf_pin").toInt();
  }
  if (server.hasArg("ppm_pin")) {
    next.ppmPin = (int8_t)server.arg("ppm_pin").toInt();
  }

  if (server.hasArg("sta_ssid")) {
    const String ssid = server.arg("sta_ssid");
    const String pass = server.arg("sta_pass");
    if (ssid.length() == 0) {
      next.staSsid[0] = '\0';
      next.staPass[0] = '\0';
    } else {
      if (ssid.length() >= sizeof(next.staSsid)) {
        sendError(400, "STA SSID is too long");
        return;
      }
      strlcpy(next.staSsid, ssid.c_str(), sizeof(next.staSsid));
      if (pass.length() > 0) {
        if (pass.length() < 8 || pass.length() >= sizeof(next.staPass)) {
          sendError(400, "STA password must be 8-64 characters");
          return;
        }
        strlcpy(next.staPass, pass.c_str(), sizeof(next.staPass));
      } else if (strlen(next.staPass) < 8) {
        sendError(400, "STA password required for the configured SSID");
        return;
      }
    }
  }

  String err;
  if (!configValidate(next, err)) {
    sendError(400, err);
    return;
  }

  const bool reboot = configNeedsReboot(old, next);
  configCommit(next);

  uint8_t revCount = 0;
  for (uint8_t i = 0; i < CH_COUNT; i++) {
    revCount += next.rev[i] ? 1 : 0;
  }
  Serial.printf("[CFG] web update: count=%u frame=%uus pulse=%uus invert=%u rev=%u fs=%s/%ums\n",
                next.count, next.frameUs, next.pulseUs, next.invert, revCount,
                next.failsafeMode == FS_HOLD ? "hold" : "preset", next.failsafeMs);
  if (reboot) {
    Serial.println("[CFG] hardware/network change -> reboot required");
  }

  String body = String("{\"ok\":true,\"reboot\":") + (reboot ? "true" : "false") + "}";
  sendJson(200, body);
  if (reboot) {
    appScheduleReboot("web configuration change", 1500);
  }
}

static void handleTest() {
  bool on = false;
  if (server.hasArg("on")) {
    const String v = server.arg("on");
    on = (v == "1" || v == "true" || v == "on");
  }
  gTestMode = on;
  Serial.printf("[TEST] mode %s\n", on ? "ON" : "OFF");
  sendJson(200, String("{\"ok\":true,\"test\":") + (on ? "true" : "false") + "}");
}

static void handleSelfTest() {
  uint16_t decoded[CH_COUNT];
  bool crcRejected = false;
  const bool pass = crsfSelfTest(decoded, crcRejected);
  String j = String("{\"ok\":true,\"pass\":") + (pass ? "true" : "false");
  j += ",\"crc_rejected\":";
  j += crcRejected ? "true" : "false";
  j += ",\"decoded\":";
  appendU16Array(j, decoded, CH_COUNT);
  j += '}';
  sendJson(200, j);
  Serial.printf("[TEST] CRSF self-test %s (crc rejected: %s)\n", pass ? "PASS" : "FAIL",
                crcRejected ? "yes" : "no");
}

static void handleReboot() {
  sendJson(200, "{\"ok\":true,\"reboot\":true}");
  appScheduleReboot("web request", 500);
}

static void handleNotFound() {
  const String uri = server.uri();
  if (uri.startsWith("/api/")) {
    sendError(404, "not found");
    return;
  }
  // The captive DNS answers every name with the AP address, so OS connectivity
  // probes arrive here. Redirect them to the portal; that is what makes the
  // "sign in to network" sheet appear on phones and desktop OSes.
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Location", portalUrl());
  server.send(302, "text/html",
              "<!doctype html><meta http-equiv=\"refresh\" content=\"0;url=/\">"
              "<a href=\"/\">CRSF2PPM config portal</a>");
}

void webSetup() {
  // WebServer drops request headers unless they are registered here.
  static const char *kCollectedHeaders[] = {"X-Lang", "Accept-Language"};
  server.collectHeaders(kCollectedHeaders, sizeof(kCollectedHeaders) / sizeof(kCollectedHeaders[0]));

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/config", HTTP_GET, handleConfigGet);
  server.on("/api/config", HTTP_POST, handleConfigPost);
  server.on("/api/test", HTTP_POST, handleTest);
  server.on("/api/selftest", HTTP_POST, handleSelfTest);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  server.onNotFound(handleNotFound);
}

void webStart() {
  if (s_webStarted) {
    return;
  }
  server.begin();
  s_webStarted = true;
  Serial.println("[WEB] HTTP server on port 80");

  // Captive portal: answer every DNS name with the AP address. The DHCP
  // server hands out this address as the client's DNS server.
  const IPAddress ap = WiFi.softAPIP();
  if (s_dns.start(53, "*", ap)) {
    Serial.printf("[WEB] captive DNS: * -> %s\n", ap.toString().c_str());
  } else {
    Serial.println("[WEB] captive DNS start failed");
  }
}

void webStop() {
  if (!s_webStarted) {
    return;
  }
  s_dns.stop();
  server.stop();
  s_webStarted = false;
  Serial.println("[WEB] HTTP server stopped");
}

void webLoop() {
  if (s_webStarted) {
    s_dns.processNextRequest();
    server.handleClient();
  }
}
