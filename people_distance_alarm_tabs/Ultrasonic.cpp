#include "Ultrasonic.h"
#include "Config.h"

static volatile uint32_t echoRiseUs = 0;
static volatile uint32_t echoFallUs = 0;
static volatile bool echoStarted = false;
static volatile bool echoDone = false;

static bool waiting = false;
static uint32_t lastCycleMs = 0;
static uint32_t triggeredAtMs = 0;

constexpr uint32_t MEASURE_INTERVAL_MS = 100;
constexpr uint32_t ECHO_TIMEOUT_MS = 30;

static void IRAM_ATTR echoIsr() {
  uint32_t t = micros();
  if (digitalRead(ECHO_PIN)) {
    echoRiseUs = t;
    echoStarted = true;
  } else if (echoStarted) {
    echoFallUs = t;
    echoDone = true;
  }
}

void ultrasonicBegin() {
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echoIsr, CHANGE);
}

bool ultrasonicUpdate(float &distanceCm) {
  uint32_t now = millis();

  if (!waiting) {
    if (now - lastCycleMs < MEASURE_INTERVAL_MS) return false;

    echoStarted = false;
    echoDone = false;

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(3);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    triggeredAtMs = now;
    waiting = true;
    return false;
  }

  if (echoDone) {
    uint32_t duration = echoFallUs - echoRiseUs;
    float d = duration * 0.0343f / 2.0f;

    waiting = false;
    lastCycleMs = now;

    distanceCm = (duration > 0 && d <= SENSOR_MAX_CM) ? d : -1.0f;
    return true;
  }

  if (now - triggeredAtMs >= ECHO_TIMEOUT_MS) {
    waiting = false;
    lastCycleMs = now;
    distanceCm = -1.0f;
    return true;
  }

  return false;
}
