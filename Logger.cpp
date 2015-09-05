#include "Logger.h"
#include "DEBUG.h"
#include "Time.h"
#include <SD.h>

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

void logger::updateStamp(const boolean stamped) {
  if(stamped) {
    sprintf(stamp, "%02d:%02d,%02d-%02d-%04d,",
      hour(),
      minute(),
      day(),
      month(),
      year()
    );
  } else stamp[0] = 0;
}

const boolean logger::logMessage(const char *fileName, const char *s, const boolean stamped) {
  boolean result = false;
  if(sdPresent) {
    File dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile) {
      updateStamp(stamped);
      dataFile.print(stamp);
      dataFile.println(s);
      dataFile.close();
      DEBUG("Data logged - " + String(stamp) + String(s));
      result = true;
    }
    dataFile.close();
  }
  return result;
}

boolean const logger::logValue(const char *fileName, const unsigned long value, const boolean stamped) {
  sprintf(message, "%lu",value);
  logMessage(fileName, message, stamped);
}

boolean const logger::logValue(const char *fileName, const long value, const boolean stamped) {
  sprintf(message, "%ld",value);
  logMessage(fileName, message, stamped);  
}

boolean const logger::logValue(const char *fileName, const double value, const boolean stamped) {
  sprintf(message, "%fL",value);
  logMessage(fileName, message, stamped);
}

boolean const logger::logValue(const char *fileName, const char value, const boolean stamped) {
  sprintf(message, "%c",value);
  logMessage(fileName, message, stamped);
}

