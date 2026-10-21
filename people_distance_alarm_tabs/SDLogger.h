#pragma once
#include <Arduino.h>
#include <RTClib.h>

bool sdLoggerBegin();   // returns false if no SD card / module found
bool sdLoggerReady();

// Appends one row to /events.csv: date,time,distance_cm,event
// Pass nullptr for 'when' if there is no RTC (date/time columns say NO_RTC).
bool sdLogDangerEntry(float distanceCm, const DateTime* when);
