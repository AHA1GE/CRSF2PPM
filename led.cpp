#include "led.h"

#include <esp32-hal-ledc.h>
#include <math.h>

#include "app.h"
#include "board_io.h"
#include "config.h"
#include "state.h"

// Set to 0 if the status LED is wired GPIO8 -> resistor -> LED -> GND
// (active-high). The default assumes the common SuperMini wiring of 3.3 V ->
// resistor -> LED -> GPIO8, i.e. the pin sinks current (active-low).
#define STATUS_LED_ACTIVE_LOW 1

#define LED_PWM_FREQ 5000
#define LED_PWM_BITS 10
#define LED_PWM_MAX ((1u << LED_PWM_BITS) - 1)

static bool s_pwmOk = false;
static TaskHandle_t s_task = nullptr;

static bool s_btnInit = false;
static bool s_btnStable = true;   // true = pressed (pin low)
static bool s_btnLastRaw = true;
static uint32_t s_btnChangeMs = 0;

static volatile uint8_t s_mode = 0;  // breath, link, wifi, error
static volatile uint32_t s_duty = 0;

static const char *MODE_NAMES[] = {"breath", "link", "wifi", "error"};

static void writeLevel(uint32_t duty) {
  if (duty > LED_PWM_MAX) {
    duty = LED_PWM_MAX;
  }
  const uint32_t out = STATUS_LED_ACTIVE_LOW ? (LED_PWM_MAX - duty) : duty;
  s_duty = out;
  if (s_pwmOk) {
    ledcWrite(STATUS_LED_PIN, out);
  } else {
    digitalWrite(STATUS_LED_PIN, out > (LED_PWM_MAX / 2) ? HIGH : LOW);
  }
}

static void pollButton(uint32_t now) {
  const bool raw = (digitalRead(WIFI_BTN_PIN) == LOW);  // BOOT button is active-low
  if (!s_btnInit) {
    s_btnInit = true;
    s_btnStable = raw;
    s_btnLastRaw = raw;
    s_btnChangeMs = now;
    return;
  }
  if (raw != s_btnLastRaw) {
    s_btnLastRaw = raw;
    s_btnChangeMs = now;
  }
  if ((uint32_t)(now - s_btnChangeMs) >= 25 && raw != s_btnStable) {
    s_btnStable = raw;
    if (raw) {
      appRequestWifiToggle();  // handled by the main loop, not this task
    }
  }
}

static void updateLed(uint32_t now) {
  const bool error = appHasError();
  const bool wifi = appWifiEnabled();
  const AppConfig cfg = configSnapshot();
  const StateSnapshot st = stateSnapshot();
  const bool link = st.lastRcMs != 0 && (uint32_t)(now - st.lastRcMs) <= cfg.failsafeMs;

  // Priority: latched fault > Wi-Fi on > CRSF link > waiting for link.
  if (error) {
    s_mode = 3;
    writeLevel((now % 200) < 100 ? LED_PWM_MAX : 0);  // 5 Hz flash
  } else if (wifi) {
    s_mode = 2;
    writeLevel((now % 1000) < 500 ? LED_PWM_MAX : 0);  // 1 Hz flash
  } else if (link) {
    s_mode = 1;
    writeLevel(LED_PWM_MAX);
  } else {
    s_mode = 0;
    const float phase = (float)(now % 1000) / 1000.0f;
    const float breath = 0.5f - 0.5f * cosf(phase * 6.2831853f);
    writeLevel((uint32_t)(breath * LED_PWM_MAX));  // 1 Hz breathing
  }
}

static void ledTask(void *) {
  for (;;) {
    const uint32_t now = millis();
    pollButton(now);
    updateLed(now);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void ledInit() {
  pinMode(WIFI_BTN_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, STATUS_LED_ACTIVE_LOW ? HIGH : LOW);
  s_pwmOk = ledcAttach(STATUS_LED_PIN, LED_PWM_FREQ, LED_PWM_BITS);
  writeLevel(0);
}

void ledStart() {
  if (s_task == nullptr) {
    xTaskCreate(ledTask, "led", 3072, nullptr, 1, &s_task);
  }
}

const char *ledModeName() {
  const uint8_t mode = s_mode;
  return MODE_NAMES[mode < 4 ? mode : 0];
}

uint32_t ledDuty() {
  return s_duty;
}

bool ledButtonPressed() {
  return s_btnStable;
}
