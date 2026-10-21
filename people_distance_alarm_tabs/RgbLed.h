#pragma once
#include <Arduino.h>

enum class Zone : uint8_t { NONE, DANGER, WARNING, SAFE };

void rgbBegin();
void rgbSet(uint8_t r, uint8_t g, uint8_t b);
void rgbOff();

// Shows the zone colour (red / yellow / green). While the rainbow is
// running the zone is remembered and shown when the rainbow ends.
void rgbShowZone(Zone zone);

void rgbStartRainbow();   // loops through all colours for 5 seconds
void rgbStopRainbow();
void rgbUpdateRainbow();  // call every loop
bool rgbRainbowActive();
