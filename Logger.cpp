#include "Logger.h"
#include "DEBUG.h"
#include <SD.h>

// boolean logger::SD_Present;

logger::logger(byte p1, byte p2, byte p3, byte p4) {
  if (SD.begin(p1, p2, p3, p4)) {
    sdPresent = true;
    DEBUG("SD card present and configured");
  } else {
    DEBUG("SD card failed, or not present");
    sdPresent = false;
  }
}

const boolean logger::sdIsPresent(void) {
  return sdPresent;
}

const boolean logger::logMessage(const char *fileName, const char *s) {
  boolean result = false;
  if(sdPresent) {
    File dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile) {
      dataFile.println(s);
      dataFile.close();
      DEBUG("Data logged - " + String(s));
      result = true;
    }
    dataFile.close();
  }
  return result;
}

