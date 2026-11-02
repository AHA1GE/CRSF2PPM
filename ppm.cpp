#include "ppm.h"

#include <Arduino.h>
#include <esp_timer.h>
#include <math.h>

#include "app.h"
#include "config.h"
#include "state.h"

static int s_pin = -1;
static TaskHandle_t s_task = nullptr;

static uint16_t rawToUs(uint16_t raw) {
  // CRSF 11-bit range: 172..1811 -> 988..2012 us, centre 992 -> 1500 us.
  int32_t us = 1500 + ((int32_t)raw - 992) * 5 / 8;
  if (us < 800) {
    us = 800;
  } else if (us > 2200) {
    us = 2200;
  }
  return (uint16_t)us;
}

static uint16_t testRaw(uint8_t channel, uint32_t now) {
  const float phase = (float)now / 4000.0f + (float)channel * 0.11f;
  int32_t v = 992 + (int32_t)(790.0f * sinf(phase * 6.2831853f));
  if (v < 172) {
    v = 172;
  } else if (v > 1811) {
    v = 1811;
  }
  return (uint16_t)v;
}

static void ppmTask(void *) {
  rmt_data_t symbols[CH_COUNT + 1];
  uint16_t hold[CH_COUNT] = {0};
  bool holdValid = false;
  int appliedInvert = -1;

  for (;;) {
    const AppConfig cfg = configSnapshot();
    const StateSnapshot st = stateSnapshot();
    const uint32_t now = millis();
    const bool test = gTestMode;

    const bool fresh =
        test || (st.lastRcMs != 0 && (uint32_t)(now - st.lastRcMs) <= cfg.failsafeMs);

    uint8_t count = cfg.count;
    if (count < 1) {
      count = 1;
    } else if (count > CH_COUNT) {
      count = CH_COUNT;
    }

    uint16_t testChannels[CH_COUNT];
    if (test) {
      for (uint8_t i = 0; i < CH_COUNT; i++) {
        testChannels[i] = testRaw(i, now);
      }
    }

    uint16_t vals[CH_COUNT];
    for (uint8_t i = 0; i < CH_COUNT; i++) {
      vals[i] = 0;
      if (i >= count) {
        continue;
      }
      uint16_t v;
      if (test) {
        v = rawToUs(testChannels[cfg.map[i]]);
      } else if (fresh) {
        v = rawToUs(st.raw[cfg.map[i]]);
      } else if (cfg.failsafeMode == FS_HOLD && holdValid) {
        v = hold[i];
      } else {
        v = cfg.failUs[i];
      }
      if (v < 800) {
        v = 800;
      } else if (v > 2200) {
        v = 2200;
      }
      vals[i] = v;
      if (fresh) {
        hold[i] = v;
      }
    }
    if (fresh) {
      holdValid = true;
    }

    uint32_t sum = 0;
    for (uint8_t i = 0; i < count; i++) {
      sum += vals[i];
    }

    // Classic PPM: N channel intervals between rising edges, then a long sync
    // interval. There are N+1 pulses per frame, including the sync pulse.
    uint32_t sync = (cfg.frameUs > sum) ? (uint32_t)(cfg.frameUs - sum) : 0;
    if (sync < 3000) {
      sync = 3000;  // Betaflight treats any interval > 2700 us as sync
    }

    const uint8_t pulseLevel = cfg.invert ? 0 : 1;
    const uint8_t gapLevel = cfg.invert ? 1 : 0;

    uint8_t n = 0;
    for (uint8_t i = 0; i < count; i++) {
      symbols[n].level0 = pulseLevel;
      symbols[n].duration0 = cfg.pulseUs;
      symbols[n].level1 = gapLevel;
      symbols[n].duration1 = (uint32_t)vals[i] - cfg.pulseUs;
      n++;
    }
    // Frame-end pulse; its low/high tail is the long sync gap.
    symbols[n].level0 = pulseLevel;
    symbols[n].duration0 = cfg.pulseUs;
    symbols[n].level1 = gapLevel;
    symbols[n].duration1 = sync - cfg.pulseUs;
    n++;

    if ((int)cfg.invert != appliedInvert) {
      rmtSetEOT(s_pin, cfg.invert ? 1 : 0);
      appliedInvert = cfg.invert;
    }

    const uint32_t t0 = (uint32_t)esp_timer_get_time();
    // Generous timeout: the frame is only ~22.5 ms, but Wi-Fi startup can
    // preempt this task briefly. A longer wait avoids a false error counter.
    const bool ok = rmtWrite(s_pin, symbols, n, 1000);
    const uint32_t t1 = (uint32_t)esp_timer_get_time();
    uint32_t duration = t1 - t0;
    if (duration > 65535) {
      duration = 65535;
    }

    stateSetOutputs(vals, (uint16_t)duration, !fresh, ok);

    if (!ok) {
      vTaskDelay(pdMS_TO_TICKS(5));
    }
  }
}

bool ppmInit(int pin) {
  s_pin = pin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  const bool ok = rmtInit(pin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 1000000);
  if (ok) {
    rmtSetEOT(pin, 0);
  }
  return ok;
}

void ppmStart() {
  if (s_task == nullptr && s_pin >= 0) {
    xTaskCreate(ppmTask, "ppm", 4096, nullptr, 3, &s_task);
  }
}
