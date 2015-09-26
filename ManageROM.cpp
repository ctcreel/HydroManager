#include "manageROM.h"
#include "Config.h"
#include "DEBUG.h"
#include "EEPROMAnything.h"

manageROM::layout manageROM::configuration;

const unsigned long manageROM::getDesiredAirTemp(void) {
  return configuration.desiredAirTemp;
}

const unsigned long manageROM::getDesiredHumidity(void) {
  return configuration.desiredHumidity;
}

const unsigned long manageROM::getDesiredHumiditySpread(void) {
  return configuration.desiredHumiditySpread;
}

const unsigned long manageROM::getMaxHumidity(void) {
  return configuration.desiredHumidity + configuration.desiredHumiditySpread;
}

const unsigned long manageROM::getDesiredMoisture(void) {
  return configuration.desiredMoisture;
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

const unsigned long manageROM::getMoistureInterval(void) {
  return configuration.moistureInterval;
}

const unsigned long manageROM::getHeightInterval(void) {
  return configuration.heightInterval;
}

const unsigned long manageROM::getTempInterval(void) {
  return configuration.tempInterval;
}

const unsigned long manageROM::getHumidityInterval(void) {
  return configuration.humidityInterval;
}

void manageROM::setDesiredAirTemp(unsigned long v) {
  configuration.desiredAirTemp = v;
  writeConfiguration();
}

void manageROM::setDesiredHumidity(unsigned long v) {
  configuration.desiredHumidity = v;
  writeConfiguration();
}

void manageROM::setDesiredHumiditySpread(unsigned long v) {
  configuration.desiredHumiditySpread = v;
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
 
unsigned long manageROM::setMoistureInterval(unsigned long v) {
  configuration.moistureInterval = v;
  writeConfiguration();
}

unsigned long manageROM::setHeightInterval(unsigned long v) {
  configuration.heightInterval = v;
  writeConfiguration();
}

unsigned long manageROM::setTempInterval(unsigned long v) {
  configuration.tempInterval = v;
  writeConfiguration();
}

unsigned long manageROM::setHumidityInterval(unsigned long v) {
  configuration.humidityInterval = v;
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
    unsigned long da,
    unsigned long dm,
    unsigned long dh,
    unsigned long dhs,
    unsigned long pot,
    unsigned long lot,
    unsigned long lst,
    unsigned long cd,
    unsigned long mi,
    unsigned long hi,
    unsigned long ti,
    unsigned long hmi) {
  DEBUG("Initializing device");
  configuration.initialized = INITIALIZATION_CODE;
  configuration.desiredAirTemp = da;
  configuration.desiredMoisture = dm;
  configuration.desiredHumidity = dh;
  configuration.pumpOnTime = pot;
  configuration.lightOnTime = lot;
  configuration.lightStartTime = lst;
  configuration.moistureInterval = mi;
  configuration.heightInterval = hi;
  configuration.tempInterval = ti;
  configuration.humidityInterval = hmi;
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
    configuration.desiredAirTemp = DESIRED_AIR_TEMP;
    configuration.desiredHumidity = DESIRED_MOISTURE;
    configuration.desiredHumiditySpread = DESIRED_HUMIDITY_SPREAD;
    /*
    configuration.desiredHumidity = DESIRED_HUMIDITY_FLOWERING;
    configuration.lightOnTime = LIGHT_ON_TIME_FLOWER;
    configuration.lightStartTime = LIGHT_START_TIME_FLOWER;
    */
    configuration.desiredHumidity = DESIRED_HUMIDITY_VEG;
    configuration.lightOnTime = LIGHT_ON_TIME_VEG;
    configuration.lightStartTime = LIGHT_START_TIME_VEG;

    configuration.pumpOnTime = PUMP_ON_TIME;
    configuration.moistureInterval = CHECK_MOISTURE_INTERVAL;
    configuration.heightInterval = CHECK_HEIGHT_INTERVAL;
    configuration.tempInterval = CHECK_TEMP_INTERVAL;
    configuration.humidityInterval = CHECK_HUMIDITY_INTERVAL;
    writeConfiguration();
}
