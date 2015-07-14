#include "DEBUG.h"
#include "Sensors.h"
#include <TimeAlarms.h>

sensor::sensor(
  byte d,
  byte p,
  unsigned int pUI,
  float hV,
  float rH,
  float rL
) {
    data = d;
    power = p;
    powerUpInterval = pUI;
    highVoltage = 1024 * (3.0/5.0); // B - output from sensor is 3 volts range for arduino is 5
    readingHigh = rH;
    readingLow = rL;
}

const float sensor::getReading(void) {
//  digitalWrite(power, HIGH);
//  Alarm.delay(powerUpInterval);
  
  float d = analogRead(data);
  float reading = 1 + (d*(readingHigh-readingLow))/(highVoltage);

//  digitalWrite(power, LOW);
  return reading;
}

