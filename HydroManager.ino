#include "sitter.h"
#include "DEBUG.h"
#include "Commands.h"
#include "Hardware.h"
#include "eventStream.h"
#include "eventHandler.h"
#include "eventCallBack.h"
#include "SensorConfig.h"
#include "manageROM.h"
#include "config.h"

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

sitter *s;
manageROM *c;
logger *loggerDevice;
RTC_DS1307 RTC;
unsigned int heightMeasurements[MEASUREMENTS];
unsigned int heightMeasures;

generatorDeviceID gID;

eventStream *e;


class environment {

};

time_t syncProvider()     //this does the same thing as RTC_DS1307::get()
{
  return RTC.now().unixtime();
}

void setup () {
  DEBUG_BEGIN(BAUD_RATE);
  Wire.begin();
  loggerDevice = new logger(SD_ONE, SD_TWO, SD_THREE, SD_FOUR);
  c = new manageROM;
  //c->reset();
  setUpClock();
  setUpSitter();
  setUpRadio();
  resetFlowerCheck();
  // Make sure we collect the height regularly
  Alarm.timerRepeat(c->getHeightInterval(), getHeight);
  DEBUG("Height Interval "+String(c->getHeightInterval()));
  // Check every day if we should switch to flowering
  Alarm.alarmRepeat(23,00,00,flowerCheck);
 }

void loop()
{  
  e->check(2);
}

void getHeight(void) {
  e->createEvent("",GET_DISTANCE);
}

void setUpSitter(void) {

    DEBUG("Light on time during setup is " + String(c->getLightOnTime()));
    
    s = new sitter(
      10, // maximum low water counts before notification
      c,
      new light(LIGHT_PIN),
      new fan(FAN_PIN),
      new pump(PUMP_PIN),
      new sensor(WATER_LEVEL_PIN,WATER_LEVEL_POWER,WATER_LEVEL_SENSOR_POWER_UP,WATER_LEVEL_SENSOR_MAX_VOLTAGE,MAX_WATER_LEVEL,MIN_WATER_LEVEL),
      new sensor(MOISTURE_PIN, MOISTURE_POWER, MOISTURE_SENSOR_POWER_UP,MOISTURE_SENSOR_MAX_VOLTAGE,MAX_MOISTURE,MIN_MOISTURE),
      new sensor(TEMP_PIN, TEMP_POWER, TEMP_SENSOR_POWER_UP,TEMP_SENSOR_MAX_VOLTAGE,MAX_TEMP,MIN_TEMP),
      loggerDevice
   );
}

void setUpRadio(void) {

   Serial3.begin(BAUD_RATE);
   e = new eventStream(&Serial3,&gID);

   new eventOutgoing(e, s->getHumidity,SET_HUMIDITY,GET_HUMIDITY);
   new eventOutgoing(e, s->getWaterLevel,SET_WATER_LEVEL,GET_WATER_LEVEL);
   new eventOutgoing(e, s->getTemp,SET_TEMP,GET_TEMP);
   new eventOutgoing(e, s->getTime,SET_TIME,GET_TIME);
   new eventOutgoing(e, s->lightIsOn,SET_LIGHT_ON,GET_LIGHT_ON);
   new eventOutgoing(e, s->fanIsOn,SET_FAN_ON,GET_FAN_ON);
   new eventOutgoing(e, s->pumpIsOn,SET_PUMP_ON,GET_PUMP_ON);
   
   /* Set up configuration settings */
  
   new eventOutgoing(e, c->getDesiredMoisture, SET_DESIRED_HUMIDITY,GET_DESIRED_HUMIDITY);
   new eventIncoming(e, c->setDesiredHumidity, SET_DESIRED_HUMIDITY);
   new eventOutgoing(e, c->getLightStartTime, SET_START_TIME, GET_START_TIME);
   new eventIncoming(e, c->setLightStartTime, SET_START_TIME);
   new eventOutgoing(e, c->getLightOnTime, SET_TIME_ON, GET_TIME_ON);
   new eventIncoming(e, c->setLightOnTime, SET_TIME_ON);
   new eventOutgoing(e, c->getCoolDownTime, SET_COOL_DOWN, GET_COOL_DOWN);
   new eventIncoming(e, c->setCoolDownTime, SET_COOL_DOWN);
   new eventOutgoing(e, c->getPumpOnTime, SET_PUMP_ON, GET_PUMP_ON);
   new eventIncoming(e, c->setPumpOnTime, SET_PUMP_ON);
   
   /* Set up remote devices */
   
   new eventIncoming(e, logDistance, SET_DISTANCE);
   new eventIncoming(e, logHeight, SET_HEIGHT);
   new eventIncoming(e, logHeightAlert, SET_DISTANCE_ALARM);

   new eventOutgoing(e, getGrowMode, SET_GROW_MODE, GET_GROW_MODE);
}

void setUpClock(void) {
  RTC.begin();
  setSyncProvider(syncProvider);     //reference our syncProvider function instead of RTC_DS1307::get()

//  for debugging purposes
//  setTime(17,15,00,1,1,2015);
//  RTC.adjust(now());
//  DEBUG("Current time is - " + String(hour()) + ":" + String(minute()) + " on " + String(month()) + "-" + String(day()) + "-" + String(year()));
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
      if(averageHeight >= flowerAtHeight && c->getLightOnTime() != LIGHT_ON_TIME_FLOWER) {
        c->setLightOnTime(LIGHT_ON_TIME_FLOWER);
        c->setLightStartTime(LIGHT_START_TIME_FLOWER);
      } else if(averageHeight < flowerAtHeight && c->getLightOnTime() != LIGHT_ON_TIME_VEG) {
        c->setLightOnTime(LIGHT_ON_TIME_VEG);
        c->setLightStartTime(LIGHT_START_TIME_VEG);
      }
      resetFlowerCheck();
    }
}

const unsigned long getGrowMode(void) {
  if(c->getLightOnTime() == LIGHT_ON_TIME_FLOWER) {
    return 1;
  } else if(c->getLightOnTime() == LIGHT_ON_TIME_VEG) {
    return 0;
  } else {
    return 3;
  }
}

void logHeight(unsigned long h) {
  char message[100];
  sprintf(message, "%02d:%02d,%02d-%02d-%04d,%d",
    hour(),
    minute(),
    day(),
    month(),
    year(),
    h);
  heightMeasurements[heightMeasures % MEASUREMENTS] = h;
  heightMeasures++;
  loggerDevice->logMessage("HEIGHT.TXT",message);
  DEBUG("Plant height is - " + String(h));
}

void logDistance(unsigned long h) {
    char message[100];
    sprintf(message, "%02d:%02d,%02d-%02d-%04d,%d",
      hour(),
      minute(),
      day(),
      month(),
      year(),
      h);
  loggerDevice->logMessage("DISTANCE.TXT",message);
  DEBUG("Distance from light is - " + String(h));
}

void logHeightAlert(unsigned long h) {
    char message[100];
    sprintf(message, "%02d:%02d,%02d-%02d-%04d,%d",
      hour(),
      minute(),
      day(),
       month(),
      year(),
      h);
  loggerDevice->logMessage("ALARM_H.TXT",message);
  Serial.print("Distance alert - ");
  Serial.println(message);
}
