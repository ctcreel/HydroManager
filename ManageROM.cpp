#include "manageROM.h"
#include "Config.h"
#include "DEBUG.h"
#include "EEPROMAnything.h"

manageROM::layout manageROM::configuration;

const unsigned long manageROM::getDesiredHumidity(void) {
  return configuration.desiredHumidity;
}

const unsigned long manageROM::getPumpOnTime(void) {
  return configuration.pumpOnTime;
}

const unsigned long manageROM::getLightStartTime(void) {
  return configuration.lightStartTime;
}

const unsigned long manageROM::getLightOnTime(void) {
  return configuration.lightOnTime;
}

const unsigned long manageROM::getCoolDownTime(void) {
  return configuration.coolDown;
}

const unsigned long manageROM::getCheckInterval(void) {
  return configuration.checkInterval;
}

void manageROM::setDesiredHumidity(unsigned long v) {
  configuration.desiredHumidity = v;
  writeConfiguration();
}

void manageROM::setPumpOnTime(unsigned long v) {
  configuration.pumpOnTime = v;
  writeConfiguration();
}

void manageROM::setLightStartTime(unsigned long v) {
  configuration.lightStartTime = v;
  writeConfiguration();
}

void manageROM::setCoolDownTime(unsigned long v) {
  configuration.coolDown = v;
  writeConfiguration();
}
 
unsigned long manageROM::setCheckInterval(unsigned long v) {
  configuration.checkInterval = v;
  writeConfiguration();
}

void manageROM::setLightOnTime(unsigned long v) {
  configuration.lightOnTime = v;
  writeConfiguration();
}

void manageROM::writeConfiguration(void) {
  EEPROM_writeAnything(0, configuration);
}

void manageROM::readConfiguration(void) {
  EEPROM_readAnything(0, configuration);
}

manageROM::manageROM(
    unsigned long dh,
    unsigned long pot,
    unsigned long lot,
    unsigned long lst,
    unsigned long cd,
    unsigned long ci) {
  DEBUG("Initializing device");
  configuration.initialized = INITIALIZATION_CODE;
  configuration.desiredHumidity = dh;
  configuration.pumpOnTime = pot;
  configuration.lightOnTime = lot;
  configuration.lightStartTime = lst;
  configuration.coolDown = cd;
  configuration.checkInterval = ci;
  writeConfiguration();
}

manageROM::manageROM(void) {
  readConfiguration();
  if(configuration.initialized != (unsigned long) INITIALIZATION_CODE) {
    reset();
  }
}

void manageROM::reset(void) {
    DEBUG("Resetting ROM");
    configuration.initialized = INITIALIZATION_CODE;
    configuration.desiredHumidity = DESIRED_HUMIDITY;
    configuration.pumpOnTime = PUMP_ON_TIME;
    configuration.lightOnTime = LIGHT_ON_TIME;
    configuration.lightStartTime = LIGHT_START_TIME;
    configuration.coolDown = COOL_DOWN;
    configuration.checkInterval = CHECK_INTERVAL;
    writeConfiguration();
}
