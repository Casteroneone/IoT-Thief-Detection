#include "SDLogger.h"
#include "Config.h"
#include <SPI.h>
#include <SD.h>

static bool ready = false;

bool sdLoggerBegin() {
  SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  ready = SD.begin(SD_CS_PIN, SPI);

  if (!ready) {
    Serial.println("WARNING: logging SD card unavailable.");
    return false;
  }

  Serial.println("Logging SD card ready.");

  if (!SD.exists(LOG_PATH)) {
    File file = SD.open(LOG_PATH, FILE_WRITE);
    if (file) {
      file.println("date,time,distance_cm,event");
      file.close();
    }
  }
  return true;
}

bool sdLoggerReady() {
  return ready;
}

bool sdLogDangerEntry(float distanceCm, const DateTime* when) {
  char dateBuf[11] = "NO_RTC";
  char timeBuf[9]  = "NO_RTC";

  if (when != nullptr) {
    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d",
             when->year(), when->month(), when->day());
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d",
             when->hour(), when->minute(), when->second());
  }

  Serial.printf("Danger entry: %s %s, %.1f cm\n", dateBuf, timeBuf, distanceCm);

  if (!ready) {
    Serial.println("WARNING: SD logging unavailable; entry not saved.");
    return false;
  }

  File file = SD.open(LOG_PATH, FILE_APPEND);
  if (!file) {
    Serial.println("ERROR: Could not open /events.csv");
    return false;
  }

  file.print(dateBuf);
  file.print(',');
  file.print(timeBuf);
  file.print(',');
  file.print(distanceCm, 1);
  file.println(",DANGER_ENTRY");
  file.close();

  Serial.println("Saved to SD card.");
  return true;
}
