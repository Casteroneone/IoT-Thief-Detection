#pragma once
#include <Arduino.h>

bool audioBegin();      // returns false if the DFPlayer is not found
bool audioReady();
void audioPlayAlarm();  // plays /01/001.mp3 from the DFPlayer card
