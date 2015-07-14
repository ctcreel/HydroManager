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
sensor *sitter::moistureSensor;
sensor *sitter::tempSensor;
light *sitter::lightDevice;
fan *sitter::fanDevice;
pump *sitter::pumpDevice;
logger *sitter::dataLogger;
byte sitter::lowWaterCount;
byte sitter::maxLowWaterCount;

/********************************/
/* Methods for the sitter class */
/********************************/

void sitter::checkAndLog(void) {
    unsigned int humidity = moistureSensor->getReading();
    unsigned int temp = tempSensor->getReading();
    if(humidity < configuration->getDesiredHumidity()) {
        pumpDevice->turnOn(configuration->getPumpOnTime());
        lowWaterCount++;
    } else {
      lowWaterCount = 0;
    }
    
    char message[100];
    sprintf(message, "%02d:%02d,%02d-%02d-%04d,%s,%s,%s,%d,%d",
      hour(),
      minute(),
      day(),
      month(),
      year(),
      pumpDevice->isOn() ? "on" : "off",
      lightDevice->isOn() ? "on" : "off",
      fanDevice->isOn() ? "on" : "off",
      humidity,
      temp);
    
    dataLogger->logMessage("datafile.txt",message);
    DEBUG("Log entry - " + String(message));
}

void sitter::turnOnLightAndFan(unsigned long timeOn) {
  lightDevice->turnOn(timeOn);
  fanDevice->turnOn(timeOn + configuration->getCoolDownTime());
}

void sitter::dailySetup(void) {
  unsigned long current = (second() + (((unsigned long) minute())*60) + (((unsigned long) hour())*3600));
  turnOnLightAndFan((configuration->getLightStartTime() + configuration->getLightOnTime()) - current);
  DEBUG("Current seconds is - "+String(current)+" Current start time is "+String(configuration->getLightStartTime()) + " End time is " + String(configuration->getLightStartTime() + configuration->getLightOnTime()));
}

sitter::sitter(
  byte mlwc,
  manageROM * c,
  light *ld,
  fan *fd,
  pump *pd,
  sensor *ms,
  sensor *ts,
  logger *dl
) {
    maxLowWaterCount = mlwc;
    lowWaterCount = 0;
    
    // Configure the components
    configuration = c;
    lightDevice = ld;
    fanDevice = fd;
    pumpDevice = pd;
    dataLogger = dl;
    moistureSensor = ms;
    tempSensor = ts;
        
    // Set up the alarms
    DEBUG("Setting up alarm for checkAndLog every " + String(configuration->getCheckInterval()) + " seconds");
    Alarm.timerRepeat(configuration->getCheckInterval(), checkAndLog);
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

const unsigned long sitter::getTemp(void) {
  return tempSensor->getReading();
}

const unsigned long sitter::getHumidity(void) {
  return moistureSensor->getReading();
}

const unsigned long sitter::getTime(void) {
  return now();
}

