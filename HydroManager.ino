// #include "sitter.h"
#include "DEBUG.h"
#include "Commands.h"
#include "Hardware.h"
#include "eventStream.h"
#include "eventHandler.h"
#include "eventCallBack.h"
#include "SensorConfig.h"
#include "manageROM.h"
#include "config.h"
#include "Sensors.h"
#include "DHT.h"
#include "Logger.h"
#include "relayArray.h"

// Third party libraries
#include <Wire.h>
#include <RTClib.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <SPI.h>
#include <SD.h>
#include <TextFinder.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>

RTC_DS1307 RTC;
unsigned int heightMeasurements[MEASUREMENTS];
unsigned int heightMeasures;

generatorDeviceID gID;
manageROM c;
relayArray array(LIGHT_PIN,FAN_PIN,PUMP_PIN,FOGGER_PIN);
sensorAnalog waterLevel(WATER_LEVEL_PIN,WATER_LEVEL_POWER,WATER_LEVEL_SENSOR_POWER_UP,MAX_WATER_LEVEL,MIN_WATER_LEVEL);
sensorAnalog moisture(MOISTURE_PIN, MOISTURE_POWER, MOISTURE_SENSOR_POWER_UP,MAX_MOISTURE,MIN_MOISTURE);
sensorAnalog soilTemp(TEMP_PIN, TEMP_POWER, TEMP_SENSOR_POWER_UP,MAX_TEMP,MIN_TEMP);
logger loggerDevice(SD_ONE, SD_TWO, SD_THREE, SD_FOUR);
eventStream e(&Serial3,&gID);

time_t syncProvider()     //this does the same thing as RTC_DS1307::get()
{
  return RTC.now().unixtime();
}

void setup () {
  // c.reset();
  DEBUG_BEGIN(BAUD_RATE);
  Wire.begin();
  Serial.begin(BAUD_RATE);

  /* Set up clock */ 
  RTC.begin();
  setSyncProvider(syncProvider);     //reference our syncProvider function instead of RTC_DS1307::get()

  /* Set up events */
  Serial3.begin(BAUD_RATE);
  
  new eventOutgoing(&e, getMoisture,SET_MOISTURE,GET_MOISTURE);
  new eventOutgoing(&e, getWaterLevel,SET_WATER_LEVEL,GET_WATER_LEVEL);
  new eventOutgoing(&e, getSoilTemp,SET_SOIL_TEMP,GET_SOIL_TEMP);
  new eventOutgoing(&e, getTime,SET_TIME,GET_TIME);
  new eventOutgoing(&e, lightIsOn,SET_LIGHT_ON,GET_LIGHT_ON);
  new eventOutgoing(&e, fanIsOn,SET_FAN_ON,GET_FAN_ON);
  new eventOutgoing(&e, pumpIsOn,SET_PUMP_ON,GET_PUMP_ON);
  
  /* Set up configuration settings */
  
  new eventOutgoing(&e, c.getDesiredMoisture, SET_DESIRED_MOISTURE,GET_DESIRED_MOISTURE);
  new eventIncoming(&e, c.setDesiredHumidity, SET_DESIRED_MOISTURE);
  new eventOutgoing(&e, c.getLightStartTime, SET_START_TIME, GET_START_TIME);
  new eventIncoming(&e, c.setLightStartTime, SET_START_TIME);
  new eventOutgoing(&e, c.getLightOnTime, SET_TIME_ON, GET_TIME_ON);
  new eventIncoming(&e, c.setLightOnTime, SET_TIME_ON);
  new eventOutgoing(&e, c.getCoolDownTime, SET_COOL_DOWN, GET_COOL_DOWN);
  new eventIncoming(&e, c.setCoolDownTime, SET_COOL_DOWN);
  new eventOutgoing(&e, c.getPumpOnTime, SET_PUMP_ON, GET_PUMP_ON);
  new eventIncoming(&e, c.setPumpOnTime, SET_PUMP_ON);
  
  /* Set up remote devices */
  
  new eventIncoming(&e, logDistance, SET_DISTANCE);
  new eventIncoming(&e, logHeight, SET_HEIGHT);
  new eventIncoming(&e, logHeightAlert, SET_DISTANCE_ALARM);
  new eventIncoming(&e, logAirTemp, SET_AIR_TEMP);
  new eventIncoming(&e, setHumidity, SET_HUMIDITY);
  
  new eventOutgoing(&e, getGrowMode, SET_GROW_MODE, GET_GROW_MODE);

  DEBUG("Light time on - " + String(c.getLightOnTime()));
  
  /* Set up alarms */
  Alarm.timerRepeat(c.getHeightInterval(), getHeight);
  Alarm.timerRepeat(60, getHumidity);
  Alarm.timerRepeat(60, getTemp);
  Alarm.timerRepeat(60, checkMoisture);
  Alarm.alarmRepeat(23,00,00,flowerCheck);
  Alarm.alarmRepeat(24,00,00,dailySetup);

  /* Let's get started */
  dailySetup();
  resetFlowerCheck();
 }

void loop()
{  
  e.check(0);
}

void dailySetup(void) {
  const unsigned long current = (second() + (((unsigned long) minute())*60) + (((unsigned long) hour())*3600));
  const unsigned long startTime = c.getLightStartTime();
  const unsigned long endTime = startTime + c.getLightOnTime();
  if(current >= 0 && current < startTime) {
    // Looks like a normal cycle so schedule wake-up alarm
    Alarm.timerOnce(startTime, dailySetup);
  } else if(current >= startTime && current < endTime) {
    // Looks like we woke up after the wake-up alarm but before the day is over
    // Turn the light on for the remainder of the day.
    unsigned long timeOn = endTime - current;
    array.turnOnOne(timeOn);
    array.turnOnTwo(timeOn + c.getCoolDownTime());
  }
}

void resetFlowerCheck(void) {
  heightMeasures = 0;
  for(unsigned int i = 0; i < MEASUREMENTS; i++) heightMeasurements[i] = 0;
}

void flowerCheck(void) {
    float flowerAtHeight = (TENT_HEIGHT - (POT_HEIGHT + LIGHT_DISTANCE + LIGHT_ASSEMBLY + LIGHT_HEIGHT))/GROWTH_AFTER_VEG;
    float dailyMeasurements = (float) heightMeasures < MEASUREMENTS ? heightMeasures : MEASUREMENTS;
    float averageHeight = 0.0;
    if(dailyMeasurements >= MIN_MEASUREMENTS) { // If we don't have enough so data don't do anything
      for(unsigned int i = 0; i < dailyMeasurements; i++) averageHeight += heightMeasurements[i];
      averageHeight /= dailyMeasurements;
      if(averageHeight >= flowerAtHeight && c.getLightOnTime() != LIGHT_ON_TIME_FLOWER) {
        c.setLightOnTime(LIGHT_ON_TIME_FLOWER);
        c.setLightStartTime(LIGHT_START_TIME_FLOWER);
        startFlowering();
      } else if(averageHeight < flowerAtHeight && c.getLightOnTime() != LIGHT_ON_TIME_VEG) {
        c.setLightOnTime(LIGHT_ON_TIME_VEG);
        c.setLightStartTime(LIGHT_START_TIME_VEG);
      }
      resetFlowerCheck();
    }
}

void startFlowering(void) {
  static unsigned int start;
  if(start < 12) {
    start++;
    Alarm.timerOnce(300, startFlowering);
    e.createEvent("",START_FLOWERING);
  } else {
    logStartFlowering();
  }
}

void setHumidity(const unsigned long h) {
  logValue("HUMIDITY.TXT",h);
  
  if((getGrowMode()==0 && h < 65) || (getGrowMode()==1 && h < 50)) {
    if(!array.isOnFour()) {
      array.turnOnFour(300);
    }
  }
}

void checkMoisture(void) {
  unsigned long moisture = getMoisture();
  unsigned long soilTemp = getSoilTemp();
  if(getMoisture() < c.getDesiredMoisture()) {
    array.turnOnThree(c.getPumpOnTime());
  }
}

/* Status Functions */

const unsigned long getGrowMode(void) {
  if(c.getLightOnTime() == LIGHT_ON_TIME_FLOWER) {
    return 1;
  } else if(c.getLightOnTime() == LIGHT_ON_TIME_VEG) {
    return 0;
  } else {
    return 3;
  }
}

void getHeight(void) {
  e.createEvent("",GET_DISTANCE);
}

void getHumidity(void) {
  e.createEvent("",GET_HUMIDITY);
}

void getTemp(void) {
  e.createEvent("",GET_AIR_TEMP);
}

const unsigned long sdIsPresent(void) {
  return loggerDevice.sdIsPresent() ? 1 : 0;
}

const unsigned long fanIsOn(void) {
  return array.isOnTwo() ? 1 : 0;
}

const unsigned long pumpIsOn(void) {
  return array.isOnThree() ? 1 : 0;
}

const unsigned long lightIsOn(void) {
  return array.isOnOne() ? 1 : 0;
}

const unsigned long getSoilTemp(void) {
  return soilTemp.getReading();
}

const unsigned long getMoisture(void) {
  return moisture.getReading();
}

const unsigned long getWaterLevel(void) {
  return waterLevel.getReading();
}

const unsigned long getTime(void) {
  return now();
}

/* Logging Functions */

void logValue(char *file, unsigned long h) {
    char message[100];
    sprintf(message, "%02d:%02d,%02d-%02d-%04d,%d",
      hour(),
      minute(),
      day(),
      month(),
      year(),
      h);
  loggerDevice.logMessage(file,message);
  DEBUG(String(file) + " logged value " + String(h));
}

void logStartFlowering(void) {
  logValue("FLOWERING.TXT",1);
}

void logHeight(unsigned long h) {
  logValue("HEIGHT.TXT",h);
  heightMeasurements[heightMeasures % MEASUREMENTS] = h;
  heightMeasures++;
}

void logDistance(unsigned long h) {
  logValue("DISTANCE.TXT",h);
}

void logHeightAlert(unsigned long h) {
  logValue("HEIGHT_ALARM.TXT",h);
}

void logAirTemp(unsigned long h) {
  logValue("AIRTEMP.TXT",h);
}

void logMoisture(unsigned long h) {
  logValue("MOISTURE.TXT",h);
}

void logSoilTemp(unsigned long h) {
  logValue("SOIL_TEMP.TXT",h);
}

void logWaterLevel(unsigned long h) {
  logValue("WATER_LEVEL.TXT",h);
}
