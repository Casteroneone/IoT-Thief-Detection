#pragma once
#include <Arduino.h>

void buttonBegin();

// Returns true once per debounced press (button wired between pin and GND).
bool buttonPressed();
