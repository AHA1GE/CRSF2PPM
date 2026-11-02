#include "crsf.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <cstring>

#include "state.h"

#define CRSF_MAX_FRAME 64
#define CRSF_RX_BUF 80

static uint8_t s_buf[CRSF_RX_BUF];
static uint8_t s_len = 0;
static SemaphoreHandle_t s_parseMux = nullptr;
static TaskHandle_t s_task = nullptr;

static uint8_t crc8(const uint8_t *ptr, uint8_t len) {
  uint8_t crc = 0;
  while (len--) {
    crc ^= *ptr++;
    for (uint8_t i = 0; i < 8; i++) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0xD5) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

static bool validAddress(uint8_t a) {
  // CRSF device addresses that a receiver may use on the wire.
  return a == 0xC8 || a == 0xEE || a == 0xEA || a == 0xEC || a == 0x00;
}

static void dropBytes(uint8_t n) {
  if (n >= s_len) {
    s_len = 0;
    return;
  }
  memmove(s_buf, s_buf + n, s_len - n);
  s_len -= n;
}

static void unpackChannels(const uint8_t *payload, uint16_t out[CH_COUNT]) {
  uint32_t bitbuf = 0;
  uint8_t bits = 0;
  uint8_t ch = 0;
  for (uint8_t i = 0; i < 22 && ch < CH_COUNT; i++) {
    bitbuf |= (uint32_t)payload[i] << bits;
    bits += 8;
    while (bits >= 11 && ch < CH_COUNT) {
      out[ch++] = (uint16_t)(bitbuf & 0x7FF);
      bitbuf >>= 11;
      bits -= 11;
    }
  }
}

static void handleFrame(const uint8_t *frame, uint8_t len) {
  // frame[0] = type, payload starts at frame[1]; len includes type and CRC.
  const uint8_t type = frame[0];
  const uint8_t payloadLen = len - 2;

  if (type == 0x16 && payloadLen >= 22) {  // RC channels packed
    uint16_t ch[CH_COUNT];
    unpackChannels(&frame[1], ch);
    stateSetChannels(ch);
  } else if (type == 0x14 && payloadLen >= 10) {  // Link statistics
    const uint8_t *p = &frame[1];
    stateSetLinkStats(-(int16_t)p[0], p[2], (int8_t)p[3]);
  }
}

static void feedUnlocked(uint8_t b) {
  if (s_len >= CRSF_RX_BUF) {
    dropBytes(1);
  }
  s_buf[s_len++] = b;

  while (s_len > 0) {
    if (!validAddress(s_buf[0])) {
      dropBytes(1);
      continue;
    }
    if (s_len < 2) {
      return;
    }
    const uint8_t len = s_buf[1];
    if (len < 2 || len > CRSF_MAX_FRAME) {
      dropBytes(1);
      continue;
    }
    if (s_len < (uint8_t)(len + 2)) {
      return;  // wait for the rest of the frame
    }
    const uint8_t expected = s_buf[len + 1];
    const uint8_t actual = crc8(&s_buf[2], len - 1);
    if (expected != actual) {
      stateCountCrcError();
      dropBytes(1);
      continue;
    }
    handleFrame(&s_buf[2], len);
    dropBytes(len + 2);
  }
}

void crsfFeedByte(uint8_t b) {
  if (s_parseMux) {
    xSemaphoreTake(s_parseMux, portMAX_DELAY);
  }
  feedUnlocked(b);
  if (s_parseMux) {
    xSemaphoreGive(s_parseMux);
  }
}

void crsfResetParser() {
  if (s_parseMux) {
    xSemaphoreTake(s_parseMux, portMAX_DELAY);
  }
  s_len = 0;
  if (s_parseMux) {
    xSemaphoreGive(s_parseMux);
  }
}

static void crsfTask(void *) {
  for (;;) {
    if (s_parseMux) {
      xSemaphoreTake(s_parseMux, portMAX_DELAY);
    }
    while (Serial1.available() > 0) {
      const int v = Serial1.read();
      if (v >= 0) {
        feedUnlocked((uint8_t)v);
      }
    }
    if (s_parseMux) {
      xSemaphoreGive(s_parseMux);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void crsfBegin(const AppConfig &cfg) {
  if (!s_parseMux) {
    s_parseMux = xSemaphoreCreateMutex();
  }
  Serial1.end();
  Serial1.setRxBufferSize(8192);
  Serial1.begin(cfg.crsfBaud, SERIAL_8N1, cfg.crsfPin, -1);
  crsfResetParser();
  if (!s_task) {
    xTaskCreate(crsfTask, "crsf", 4096, nullptr, 2, &s_task);
  }
  Serial.printf("[CRSF] UART1 RX=GPIO%d baud=%lu 8N1\n", cfg.crsfPin,
                (unsigned long)cfg.crsfBaud);
}

static void packChannels(const uint16_t in[CH_COUNT], uint8_t payload[22]) {
  memset(payload, 0, 22);
  uint32_t bitbuf = 0;
  uint8_t bits = 0;
  uint8_t idx = 0;
  for (uint8_t ch = 0; ch < CH_COUNT; ch++) {
    bitbuf |= (uint32_t)(in[ch] & 0x7FF) << bits;
    bits += 11;
    while (bits >= 8) {
      payload[idx++] = (uint8_t)(bitbuf & 0xFF);
      bitbuf >>= 8;
      bits -= 8;
    }
  }
  if (bits > 0) {
    payload[idx++] = (uint8_t)(bitbuf & 0xFF);
  }
}

// Bench-only: feed a known-good and a corrupted CRSF frame straight into the
// parser, bypassing the UART. Verifies CRC, unpacking and channel mapping state.
bool crsfSelfTest(uint16_t decoded[CH_COUNT], bool &crcRejected) {
  uint16_t sent[CH_COUNT];
  for (uint8_t i = 0; i < CH_COUNT; i++) {
    sent[i] = (uint16_t)(172 + i * 64);  // 172 .. 1132
  }

  uint8_t payload[22];
  packChannels(sent, payload);

  uint8_t frame[26];
  frame[0] = 0xC8;                             // flight controller address
  frame[1] = 24;                               // type + payload + CRC
  frame[2] = 0x16;                             // RC channels packed
  memcpy(&frame[3], payload, 22);
  frame[25] = crc8(&frame[2], 23);             // CRC over type + payload

  if (s_parseMux) {
    xSemaphoreTake(s_parseMux, portMAX_DELAY);
  }
  s_len = 0;  // parser mutex is held; do not call crsfResetParser() here
  for (uint8_t i = 0; i < sizeof(frame); i++) {
    feedUnlocked(frame[i]);
  }
  if (s_parseMux) {
    xSemaphoreGive(s_parseMux);
  }

  StateSnapshot st = stateSnapshot();
  bool match = true;
  for (uint8_t i = 0; i < CH_COUNT; i++) {
    decoded[i] = st.raw[i];
    if (st.raw[i] != sent[i]) {
      match = false;
    }
  }

  const uint32_t errBefore = st.crcErrors;
  frame[25] ^= 0xFF;
  if (s_parseMux) {
    xSemaphoreTake(s_parseMux, portMAX_DELAY);
  }
  for (uint8_t i = 0; i < sizeof(frame); i++) {
    feedUnlocked(frame[i]);
  }
  if (s_parseMux) {
    xSemaphoreGive(s_parseMux);
  }
  const uint32_t errAfter = stateSnapshot().crcErrors;
  crcRejected = errAfter > errBefore;

  return match && crcRejected;
}
