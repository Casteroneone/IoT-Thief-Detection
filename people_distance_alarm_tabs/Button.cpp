#include "Button.h"
#include "Config.h"

static bool lastRead = HIGH;
static bool lastStable = HIGH;
static uint32_t changedAtMs = 0;

void buttonBegin() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // pressed = LOW
  lastRead = digitalRead(BUTTON_PIN);
  lastStable = lastRead;
}

bool buttonPressed() {
  bool reading = digitalRead(BUTTON_PIN);
  uint32_t now = millis();

  if (reading != lastRead) {
    lastRead = reading;
    changedAtMs = now;
  }

  if ((now - changedAtMs) >= BUTTON_DEBOUNCE_MS && reading != lastStable) {
    lastStable = reading;
    if (reading == LOW) return true;  // new press
  }
  return false;
}
