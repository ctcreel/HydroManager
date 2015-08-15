#include "sitter.h"
#include <DEBUG.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <TimeAlarms.h>

/************************************/
/* Definitions for static variables */
/************************************/
manageROM *sitter::configuration;
sensor *sitter::levelSensor;
sensor *sitter::moistureSensor;
sensor *sitter::tempSensor;
light *sitter::lightDevice;
fan *sitter::fanDevice;
pump *sitter::pumpDevice;
logger *sitter::dataLogger;
byte sitter::lowWaterCount;
byte sitter::maxLowWaterCount;
bool sitter::wakeUpAlarmSet;

/********************************/
/* Methods for the sitter class */
/********************************/

void sitter::checkAndLog(void) {
    unsigned int moisture = getMoisture();
    unsigned int soilTemp = getSoilTemp();
    unsigned int resevoirLevel = getWaterLevel();
    if(moisture < configuration->getDesiredMoisture()) {
        pumpDevice->turnOn(configuration->getPumpOnTime());
        lowWaterCount++;
    } else {
      lowWaterCount = 0;
    }
    
    char message[100];
    sprintf(message, "%02d:%02d,%02d-%02d-%04d,%s,%s,%s,%d,%d,%d",
      hour(),
      minute(),
      day(),
      month(),
      year(),
      pumpDevice->isOn() ? "on" : "off",
      lightDevice->isOn() ? "on" : "off",
      fanDevice->isOn() ? "on" : "off",
      moisture,
      soilTemp,
      resevoirLevel);
    
    dataLogger->logMessage("datafile.txt",message);
    DEBUG("Log entry - " + String(message));
}

void sitter::turnOnLightAndFan(unsigned long timeOn) {
  lightDevice->turnOn(timeOn);
  fanDevice->turnOn(timeOn + configuration->getCoolDownTime());
}

void sitter::dailySetup(void) {
  const unsigned long current = (second() + (((unsigned long) minute())*60) + (((unsigned long) hour())*3600));
  const unsigned long startTime = configuration->getLightStartTime();
  const unsigned long endTime = startTime + configuration->getLightOnTime();
  if(current >= 0 && current < startTime) {
    // Looks like a normal cycle so schedule wake-up alarm
    Alarm.timerOnce(startTime, dailySetup);
  } else if(current >= startTime && current < endTime) {
    // Looks like we woke up after the wake-up alarm but before the day is over
    // Turn the light on for the remainder of the day.
    turnOnLightAndFan(endTime - current);
  }
}

sitter::sitter(
  byte mlwc,
  manageROM * c,
  light *ld,
  fan *fd,
  pump *pd,
  sensor *wl,
  sensor *ms,
  sensor *ts,
  logger *dl
) {
    maxLowWaterCount = mlwc;
    lowWaterCount = 0;
    wakeUpAlarmSet = false;
    
    // Configure the components
    configuration = c;
    lightDevice = ld;
    fanDevice = fd;
    pumpDevice = pd;
    dataLogger = dl;
    levelSensor = wl;
    moistureSensor = ms;
    tempSensor = ts;
        
    // Set up the alarms
    DEBUG("Setting up alarm for checkAndLog every " + String(configuration->getMoistureInterval()) + " seconds");
    Alarm.timerRepeat(configuration->getMoistureInterval(), checkAndLog);
    Alarm.alarmRepeat(24,00,00,dailySetup);
    dailySetup();
}

const unsigned long sitter::sdIsPresent(void) {
  return dataLogger->sdIsPresent() ? 1 : 0;
}

const unsigned long sitter::fanIsOn(void) {
  return fanDevice->isOn() ? 1 : 0;
}

const unsigned long sitter::pumpIsOn(void) {
  return pumpDevice->isOn() ? 1 : 0;
}

const unsigned long sitter::waterIsLow(void) {
  return lowWaterCount > maxLowWaterCount ? 1 : 0;
}

const unsigned long sitter::lightIsOn(void) {
  return lightDevice->isOn() ? 1 : 0;
}

const unsigned long sitter::getSoilTemp(void) {
  return tempSensor->getReading();
}

const unsigned long sitter::getMoisture(void) {
  return moistureSensor->getReading();
}

const unsigned long sitter::getTime(void) {
  return now();
}

const unsigned long sitter::getWaterLevel(void) {
  return levelSensor->getReading();
}

