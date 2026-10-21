/*
  ESP32 People-Distance Alarm System (multi-tab version)

  Tabs:
    people_distance_alarm_tabs.ino  main logic: ultrasonic + RTC + loop
    Config.h                        pins, zones, IR codes, settings
    IRControl.h / .cpp              IR remote: wake, sound, rainbow, light sleep
    RgbLed.h / .cpp                 RGB LED zones + 5 second rainbow loop
    Audio.h / .cpp                  DFPlayer alarm sound
    SDLogger.h / .cpp               /events.csv logging on the SD module
    Ultrasonic.h / .cpp             non-blocking HC-SR04P distance reading
    Button.h / .cpp                 push button on GPIO 13 = go to light sleep

  IR commands:
    BA45FF00 = wake from light sleep and enable the system
    B946FF00 = play alarm sound
    B847FF00 = rainbow loop for 5 seconds
    BB44FF00 = go back to light sleep

  Danger zone (0-20 cm): alarm sound + one row saved to SD card per entry.
  Green stage (> 80 cm) for 10 seconds: automatic light sleep.

  Libraries: RTClib (Adafruit), DFRobotDFPlayerMini, IRremoteESP8266
  Core: Arduino-ESP32 3.x
*/

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

#include "Config.h"
#include "IRControl.h"
#include "RgbLed.h"
#include "Audio.h"
#include "SDLogger.h"
#include "Ultrasonic.h"
#include "Button.h"

RTC_DS3231 rtc;
bool rtcReady = false;

bool wasInDangerZone = false;
bool alarmStartedForCurrentZone = false;

void handleDangerEntry(float distance) {
  audioPlayAlarm();  // sound first so there is no delay from SD writing

  if (rtcReady) {
    DateTime now = rtc.now();
    sdLogDangerEntry(distance, &now);
  } else {
    sdLogDangerEntry(distance, nullptr);
  }
}

// Goes to light sleep after the green stage lasts GREEN_SLEEP_MS.
bool greenTimerRunning = false;
uint32_t greenSince = 0;

void checkAutoSleep(bool inGreen) {
  // Not green, or the rainbow demo is running: restart the timer.
  if (!inGreen || rgbRainbowActive()) {
    greenTimerRunning = false;
    return;
  }

  uint32_t now = millis();

  if (!greenTimerRunning) {
    greenTimerRunning = true;
    greenSince = now;
    return;
  }

  if (now - greenSince >= GREEN_SLEEP_MS) {
    Serial.println("Green stage for 10 seconds: going to light sleep.");
    greenTimerRunning = false;
    irRequestSleep();
  }
}

void updateDistanceBehavior() {
  float distance;
  if (!ultrasonicUpdate(distance)) return;  // non-blocking, ~100 ms per reading

  if (distance < 0) {
    Serial.println("No valid ultrasonic echo.");
    checkAutoSleep(NO_ECHO_COUNTS_AS_GREEN);
    return;
  }

  Serial.printf("Distance: %.1f cm\n", distance);

  bool inDangerZone = (distance >= DANGER_MIN_CM && distance <= DANGER_MAX_CM);

  // Sound + log once each time someone enters 0-20 cm.
  if (inDangerZone && !wasInDangerZone) {
    handleDangerEntry(distance);
  }
  wasInDangerZone = inDangerZone;

  if (inDangerZone) {
    rgbShowZone(Zone::DANGER);
  } else if (distance <= WARNING_MAX_CM) {
    rgbShowZone(Zone::WARNING);
  } else {
    rgbShowZone(Zone::SAFE);
  }

  checkAutoSleep(distance > WARNING_MAX_CM);
}

void setup() {
  Serial.begin(115200);
  delay(300);

  ultrasonicBegin();
  buttonBegin();
  rgbBegin();

  // RTC
  Wire.begin(RTC_SDA_PIN, RTC_SCL_PIN);
  rtcReady = rtc.begin();

  if (rtcReady) {
    Serial.println("DS3231 detected.");
    if (rtc.lostPower()) {
      Serial.println("RTC lost power; setting clock to compile time.");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    Serial.println("WARNING: DS3231 not detected.");
  }

  sdLoggerBegin();
  audioBegin();
  irBegin();

  Serial.println();
  Serial.println("People-distance alarm initialized.");
  Serial.println("Press BA45FF00 to wake and enable.");
}

void loop() {
  if (!systemIsEnabled()) {
    irSleepUntilWake();       // returns once the wake button enables the system
    wasInDangerZone = false;  // so the first reading can log an entry
    greenTimerRunning = false;
    return;
  }

  if (buttonPressed()) {
    Serial.println("Button pressed: going to light sleep.");
    irRequestSleep();
    return;
  }

  irUpdate();
  rgbUpdateRainbow();
  updateDistanceBehavior();

  delay(1);
}
