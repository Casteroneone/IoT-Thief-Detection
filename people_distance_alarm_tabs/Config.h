#pragma once
#include <Arduino.h>

// ======================= PIN CONFIGURATION =======================

// I2C: DS3231 RTC
constexpr uint8_t RTC_SDA_PIN = 21;
constexpr uint8_t RTC_SCL_PIN = 22;

// SD logging module (VSPI)
constexpr uint8_t SD_SCK_PIN  = 18;
constexpr uint8_t SD_MISO_PIN = 19;
constexpr uint8_t SD_MOSI_PIN = 23;
constexpr uint8_t SD_CS_PIN   = 5;

// DFPlayer Mini (UART2)
constexpr uint8_t DF_RX_PIN = 16;  // ESP32 RX2 <- DFPlayer TX
constexpr uint8_t DF_TX_PIN = 17;  // ESP32 TX2 -> DFPlayer RX (via 1k resistor)

// Ultrasonic HC-SR04P
constexpr uint8_t TRIG_PIN = 25;
constexpr uint8_t ECHO_PIN = 26;

// IR receiver (RTC-capable pin, used for light-sleep wake)
constexpr uint8_t IR_PIN = 27;

// Sleep button (one leg to this pin, other leg to GND, no resistor needed)
constexpr uint8_t BUTTON_PIN = 13;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;

// RGB LED, common cathode
constexpr uint8_t LED_R_PIN = 32;
constexpr uint8_t LED_G_PIN = 33;
constexpr uint8_t LED_B_PIN = 4;

// ======================= DISTANCE ZONES =======================

constexpr float DANGER_MIN_CM  = 0.0;    // danger zone = 0 - 20 cm
constexpr float DANGER_MAX_CM  = 20.0;
constexpr float WARNING_MAX_CM = 80.0;
constexpr float SENSOR_MAX_CM  = 400.0;

// ======================= AUTO SLEEP =======================

// If the ultrasonic stays in the green stage (farther than WARNING_MAX_CM)
// for this long, the system goes to light sleep. Wake with BA45FF00.
constexpr uint32_t GREEN_SLEEP_MS = 10000;

// true  = "no echo" (nothing within range) also counts as green
// false = only a valid reading above WARNING_MAX_CM counts as green
constexpr bool NO_ECHO_COUNTS_AS_GREEN = true;

// ======================= AUDIO / LED / LOG =======================

constexpr uint8_t  DF_VOLUME            = 25;    // 0-30
constexpr uint32_t RAINBOW_DURATION_MS  = 5000;  // rainbow loop length
constexpr char     LOG_PATH[]           = "/events.csv";

// ======================= IR REMOTE =======================

// Codes are written in the "BA45FF00" style (as printed by the Arduino
// IRremote library). IRControl.cpp converts them automatically to the
// format that IRremoteESP8266 reports, so either style works.
const uint64_t CMD_WAKE    = 0xBA45FF00ULL;  // wake from light sleep + enable
const uint64_t CMD_SOUND   = 0xB946FF00ULL;  // play alarm sound
const uint64_t CMD_RAINBOW = 0xB847FF00ULL;  // rainbow loop for 5 seconds
const uint64_t CMD_SLEEP   = 0xBB44FF00ULL;  // go back to light sleep

// After a GPIO wake, how long to listen for the full IR frame.
constexpr uint32_t IR_WAKE_LISTEN_MS = 250;

// The pulse that wakes the ESP32 can be cut short, so the code may not decode.
// true  = if the frame can't be decoded, wake anyway (reliable)
// false = only wake when BA45FF00 is decoded (strict; may never wake if
//         the wake pulse is always truncated)
constexpr bool WAKE_ON_UNDECODABLE = true;
