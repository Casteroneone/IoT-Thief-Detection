#include "Audio.h"
#include "Config.h"
#include <DFRobotDFPlayerMini.h>

static HardwareSerial dfSerial(2);
static DFRobotDFPlayerMini player;
static bool ready = false;

bool audioBegin() {
  dfSerial.begin(9600, SERIAL_8N1, DF_RX_PIN, DF_TX_PIN);

  if (player.begin(dfSerial)) {
    ready = true;
    player.volume(DF_VOLUME);
    Serial.println("DFPlayer ready.");
  } else {
    ready = false;
    Serial.println("WARNING: DFPlayer initialization failed.");
  }
  return ready;
}

bool audioReady() {
  return ready;
}

void audioPlayAlarm() {
  if (!ready) {
    Serial.println("ERROR: DFPlayer is not ready.");
    return;
  }
  player.playFolder(1, 1);  // /01/001.mp3
  Serial.println("Playing alarm: /01/001.mp3");
}
