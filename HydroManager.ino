#include "sitter.h"
#include "DEBUG.h"
#include "Commands.h"
#include "Hardware.h"
#include "eventStream.h"
#include "eventHandler.h"
#include "eventCallBack.h"
#include "SensorConfig.h"
#include "manageROM.h"

// Third party libraries
#include <Wire.h>
#include <RTClib.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <TextFinder.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>

sitter *s;
manageROM *c;
logger *loggerDevice;
unsigned long lastNotified;
RTC_DS1307 RTC;

generatorDeviceID gID;
SoftwareSerial ss(11,10);

eventStream *e;

time_t syncProvider()     //this does the same thing as RTC_DS1307::get()
{
  return RTC.now().unixtime();
}

void setup () {
  DEBUG_BEGIN(BAUD_RATE);
  Wire.begin();
  setUpClock();
  setUpSitter();
  setUpRadio();
  loggerDevice = new logger(SD_ONE, SD_TWO, SD_THREE, SD_FOUR);
  lastNotified = 0;
 }

void loop()
{  
  e->check(2);
  if(s->waterIsLow()) {
    if(now() - lastNotified > 600) {
      e->createEvent("", LOW_WATER_ALARM);
      lastNotified = now();
    }
  } else {
      lastNotified = 0;
  }
}

void setUpSitter(void) {

    c = new manageROM();
    // c->reset();
    DEBUG("Light on time during setup is " + String(c->getLightOnTime()));
    
    s = new sitter(
      10, // maximum low water counts before notification
      c,
      new light(LIGHT_PIN),
      new fan(FAN_PIN),
      new pump(PUMP_PIN),
      new sensor(MOISTURE_PIN, MOISTURE_POWER, MOISTURE_SENSOR_POWER_UP,MOISTURE_SENSOR_MAX_VOLTAGE,MAX_MOISTURE,MIN_MOISTURE),
      new sensor(TEMP_PIN, TEMP_POWER, TEMP_SENSOR_POWER_UP,TEMP_SENSOR_MAX_VOLTAGE,MAX_TEMP,MIN_TEMP),
      loggerDevice
   );
}

void setUpRadio(void) {

   ss.begin(BAUD_RATE);
   e = new eventStream(&ss,&gID);

   new eventOutgoing(e, s->getHumidity,SET_HUMIDITY,GET_HUMIDITY);
   new eventOutgoing(e, s->getTemp,SET_TEMP,GET_TEMP);
   new eventOutgoing(e, s->getTime,SET_TIME,GET_TIME);
   new eventOutgoing(e, s->lightIsOn,SET_LIGHT_ON,GET_LIGHT_ON);
   new eventOutgoing(e, s->fanIsOn,SET_FAN_ON,GET_FAN_ON);
   new eventOutgoing(e, s->pumpIsOn,SET_PUMP_ON,GET_PUMP_ON);
   
   /* Set up configuration settings */
  
   new eventOutgoing(e, c->getDesiredHumidity, SET_DESIRED_HUMIDITY,GET_DESIRED_HUMIDITY);
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
   new eventIncoming(e, logHeightAlert, SET_DISTANCE_ALARM);
}

void setUpClock(void) {
  RTC.begin();
  setSyncProvider(syncProvider);     //reference our syncProvider function instead of RTC_DS1307::get()

//  for debugging purposes
//  setTime(17,15,00,1,1,2015);
//  RTC.adjust(now());
//  DEBUG("Current time is - " + String(hour()) + ":" + String(minute()) + " on " + String(month()) + "-" + String(day()) + "-" + String(year()));
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
  loggerDevice->logMessage("HEIGHT.TXT",message);
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
