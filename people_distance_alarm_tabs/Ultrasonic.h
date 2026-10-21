#pragma once
#include <Arduino.h>

void ultrasonicBegin();

// Non-blocking distance measurement. Call every loop.
// Returns true when a new result is ready:
//   distanceCm >= 0  -> valid distance
//   distanceCm = -1  -> no valid echo
// Measures roughly every 100 ms without blocking the loop,
// so the IR receiver is never starved.
bool ultrasonicUpdate(float &distanceCm);
