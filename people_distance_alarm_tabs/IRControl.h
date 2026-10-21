#pragma once
#include <Arduino.h>

void irBegin();

// True once the wake command (or a wake from light sleep) has enabled the system.
bool systemIsEnabled();

// Call every loop while enabled. Handles:
//   BA45FF00 -> enable system
//   B946FF00 -> play alarm sound
//   B847FF00 -> rainbow loop for 5 seconds
void irUpdate();

// Light sleep until the wake button is pressed. Returns with the system enabled.
void irSleepUntilWake();

// Puts the system into the disabled state; the main loop then calls
// irSleepUntilWake(). Used by the green-stage auto sleep.
void irRequestSleep();
