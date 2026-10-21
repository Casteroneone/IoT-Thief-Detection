#include "IRControl.h"
#include "Config.h"
#include "Audio.h"
#include "RgbLed.h"

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include "esp_sleep.h"
#include "driver/gpio.h"

static IRrecv irrecv(IR_PIN);
static decode_results results;
static bool enabled = false;

// ---------------------------------------------------------------
// Code format conversion.
// The Arduino "IRremote" library prints NEC codes like BA45FF00
// (each byte sent LSB-first, first byte in the low position).
// IRremoteESP8266 reports the same button as 0x00FFA25D, which is
// each byte bit-reversed and the byte order flipped.
// ---------------------------------------------------------------

static uint8_t rev8(uint8_t b) {
  uint8_t r = 0;
  for (uint8_t i = 0; i < 8; i++) {
    r = (r << 1) | (b & 1);
    b >>= 1;
  }
  return r;
}

// "BA45FF00" style  <->  IRremoteESP8266 style (the swap is symmetric)
static uint32_t swapFormat(uint32_t v) {
  return ((uint32_t)rev8(v & 0xFF) << 24) |
         ((uint32_t)rev8((v >> 8) & 0xFF) << 16) |
         ((uint32_t)rev8((v >> 16) & 0xFF) << 8) |
         (uint32_t)rev8((v >> 24) & 0xFF);
}

// True if the received value equals 'code' in either format.
static bool matches(uint64_t value, uint64_t code) {
  return value == code || value == (uint64_t)swapFormat((uint32_t)code);
}

// ---------------------------------------------------------------

void irBegin() {
  irrecv.enableIRIn();
}

bool systemIsEnabled() {
  return enabled;
}

void irRequestSleep() {
  enabled = false;
}

static void handleCommand(uint64_t command) {
  Serial.printf("IR received: 0x%llX  (BA45FF00-style: 0x%08lX)\n",
                (unsigned long long)command,
                (unsigned long)swapFormat((uint32_t)command));

  if (matches(command, CMD_WAKE)) {
    enabled = true;
    Serial.println("System enabled by remote (BA45FF00).");
    return;
  }

  if (!enabled) {
    Serial.println("System disabled; command ignored.");
    return;
  }

  if (matches(command, CMD_SOUND)) {
    audioPlayAlarm();
  } else if (matches(command, CMD_RAINBOW)) {
    rgbStartRainbow();
  } else if (matches(command, CMD_SLEEP)) {
    Serial.println("Sleep command received.");
    enabled = false;  // main loop will call irSleepUntilWake()
  } else {
    Serial.println("Unknown IR command (copy the BA45FF00-style code above to use it).");
  }
}

void irUpdate() {
  if (irrecv.decode(&results)) {
    if (results.decode_type == NEC && results.bits == 32) {
      // Ignore NEC repeat frames so a held button doesn't retrigger.
      if (results.value != 0xFFFFFFFFULL) {
        handleCommand(results.value);
      }
    } else {
      // Noise or an unsupported signal: never act on it.
      Serial.printf("Ignored non-NEC frame (%s, %u bits).\n",
                    typeToString(results.decode_type).c_str(),
                    (unsigned)results.bits);
    }
    irrecv.resume();
  }
}

void irSleepUntilWake() {
  rgbStopRainbow();
  rgbOff();
  Serial.println("Entering light sleep.");
  Serial.println("Press the wake button (BA45FF00) to enable.");
  Serial.flush();

  while (!enabled) {
    // IR receiver output idles HIGH and goes LOW during a pulse.
    gpio_wakeup_enable((gpio_num_t)IR_PIN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_light_sleep_start();
    gpio_wakeup_disable((gpio_num_t)IR_PIN);

    // Listen for the rest of the frame and check which button it was.
    irrecv.resume();

    bool     gotNec = false;
    uint64_t value  = 0;
    uint32_t start  = millis();

    while (millis() - start < IR_WAKE_LISTEN_MS) {
      if (irrecv.decode(&results)) {
        if (results.decode_type == NEC) {
          gotNec = true;
          value  = results.value;
        }
        irrecv.resume();
        if (gotNec) break;
      }
      delay(1);
    }

    if (gotNec) {
      if (matches(value, CMD_WAKE)) {
        enabled = true;
        Serial.println("Woke from light sleep: BA45FF00.");
      } else if (value != 0xFFFFFFFFULL) {
        Serial.printf("Other button 0x%llX ignored; back to sleep.\n",
                      (unsigned long long)value);
        Serial.flush();
      }
    } else if (WAKE_ON_UNDECODABLE) {
      enabled = true;
      Serial.println("Woke from light sleep (frame not decodable).");
    } else {
      Serial.println("Undecodable pulse ignored; back to sleep.");
      Serial.flush();
    }
  }

  irrecv.resume();
}
