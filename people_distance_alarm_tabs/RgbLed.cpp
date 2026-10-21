#include "RgbLed.h"
#include "Config.h"

static bool     rainbowActive     = false;
static uint32_t rainbowStartedAt  = 0;
static uint32_t rainbowLastUpdate = 0;
static uint16_t rainbowHue        = 0;
static Zone     lastZone          = Zone::NONE;

void rgbBegin() {
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  rgbOff();
}

void rgbSet(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(LED_R_PIN, r);
  analogWrite(LED_G_PIN, g);
  analogWrite(LED_B_PIN, b);
}

void rgbOff() {
  rgbSet(0, 0, 0);
}

void rgbShowZone(Zone zone) {
  lastZone = zone;
  if (rainbowActive) return;

  switch (zone) {
    case Zone::DANGER:  rgbSet(255, 0, 0);   break;
    case Zone::WARNING: rgbSet(255, 180, 0); break;
    case Zone::SAFE:    rgbSet(0, 255, 0);   break;
    default:            rgbOff();            break;
  }
}

// hue: 0-359
static void setRainbowHue(uint16_t hue) {
  hue %= 360;

  uint8_t region    = hue / 60;
  uint8_t remainder = (hue % 60) * 255 / 60;
  uint8_t q = 255 - remainder;
  uint8_t t = remainder;

  switch (region) {
    case 0:  rgbSet(255, t, 0); break;
    case 1:  rgbSet(q, 255, 0); break;
    case 2:  rgbSet(0, 255, t); break;
    case 3:  rgbSet(0, q, 255); break;
    case 4:  rgbSet(t, 0, 255); break;
    default: rgbSet(255, 0, q); break;
  }
}

void rgbStartRainbow() {
  rainbowActive     = true;
  rainbowStartedAt  = millis();
  rainbowLastUpdate = 0;
  rainbowHue        = 0;
  Serial.println("Rainbow loop started (5 seconds).");
}

void rgbStopRainbow() {
  rainbowActive = false;
}

void rgbUpdateRainbow() {
  if (!rainbowActive) return;

  uint32_t now = millis();

  if (now - rainbowStartedAt >= RAINBOW_DURATION_MS) {
    rainbowActive = false;
    Serial.println("Rainbow loop finished.");
    rgbShowZone(lastZone);  // return to the distance colour
    return;
  }

  if (now - rainbowLastUpdate >= 20UL) {
    rainbowLastUpdate = now;
    setRainbowHue(rainbowHue);
    rainbowHue = (rainbowHue + 4) % 360;  // full colour wheel about every 1.8 s
  }
}

bool rgbRainbowActive() {
  return rainbowActive;
}
