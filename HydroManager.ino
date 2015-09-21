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

generatorDeviceID gID;
manageROM c;
relayArray array(LIGHT_PIN,FAN_PIN,PUMP_PIN,FOGGER_PIN);
sensorAnalog moisture(MOISTURE_PIN, MOISTURE_POWER, MOISTURE_SENSOR_POWER_UP,MAX_MOISTURE,MIN_MOISTURE);
eventStream e(&Serial3,&gID);

unsigned int heightMeasurements[100];
unsigned int heightMeasures;

unsigned long lastTempNotification;

RTC_DS1307 RTC;
time_t syncProvider()     //this does the same thing as RTC_DS1307::get()
{
  return RTC.now().unixtime();
}

void setup () {
  DEBUG_BEGIN(BAUD_RATE);
  DEBUG("Starting up!");
  
  // c.reset(); // reset flash settings
  Wire.begin();
  Serial.begin(BAUD_RATE);

  /* Set up clock */ 
  RTC.begin();
  setSyncProvider(syncProvider);     //reference our syncProvider function instead of RTC_DS1307::get()
  DEBUG("Current time is - " + String(hour()) + ":" + String(minute()) + " on " + String(month()) + "-" + String(day()) + "-" + String(year()));

  /* Set up events */
  Serial3.begin(BAUD_RATE);

  /* Set up status checks */
  
  new eventOutgoing(&e, getMoisture,SET_MOISTURE,GET_MOISTURE);
  new eventOutgoing(&e, getTime,SET_TIME,GET_TIME);
  new eventOutgoing(&e, lightIsOn,SET_LIGHT_ON,GET_LIGHT_ON);
  new eventOutgoing(&e, fanIsOn,SET_FAN_ON,GET_FAN_ON);
  new eventOutgoing(&e, pumpIsOn,SET_PUMP_ON,GET_PUMP_ON);
  new eventOutgoing(&e, foggerIsOn,SET_FOGGER_ON,GET_FOGGER_ON);
  
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
  new eventIncoming(&e, setHumidity, SET_HUMIDITY);
  new eventIncoming(&e, setTemp, SET_AIR_TEMP);
  new eventIncoming(&e, setHeight, SET_HEIGHT);

  DEBUG("Light time on - " + String(c.getLightOnTime()));
  DEBUG("Light start time - " + String(c.getLightStartTime()));
  DEBUG("Grow mode is - " + String(getGrowMode()));

  resetHeightMeasurements();
  lastTempNotification = now();
  
  /* Set up alarms */
  Alarm.timerRepeat(c.getHeightInterval(), getHeight);
  Alarm.timerRepeat(60, getHumidity);
  Alarm.timerRepeat(60, getTemp);
  Alarm.timerRepeat(60, checkMoisture);
  Alarm.alarmRepeat(24,00,00,dailySetup);

  /* Let's get started */
  getHeight();
  getHumidity();
  getTemp();
  checkMoisture();
  dailySetup();
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
    array.turnOnOne(timeOn); // Turn on light
  }
  flowerCheck();
}

void setHumidity(const unsigned long h) {

  if((getGrowMode()==0 && h < 70) || (getGrowMode()==1 && h < 50)) {
    array.turnOnFour(); // turn on fogger
    e.createEvent("1",SET_FOGGER_ON);
  } else if(array.isOnFour()) {
    array.turnOffFour(); // turn off fogger
    e.createEvent("0",SET_FOGGER_ON);
  }
  
  if(!array.isOnOne()) { // if the light is on
    if((getGrowMode()==0 && h >= 75) || (getGrowMode()==1 && h >= 55)) {
      // and the humidity is too high
      array.turnOnTwo(); // turn on fan
      e.createEvent("1",SET_FAN_ON);
    } else if(array.isOnTwo()) {
      array.turnOffTwo(); // turn off fan
      e.createEvent("0",SET_FAN_ON);
    }
  }
}

void checkMoisture(void) {
  unsigned long moisture = getMoisture();
  DEBUG(String("Moisture is ")+String(moisture));
  if(moisture < c.getDesiredMoisture()) {
    array.turnOnThree(c.getPumpOnTime());
  }
  e.createEvent(moisture,SET_MOISTURE);
}

/* Flowering Controls */

void resetHeightMeasurements(void) {
  heightMeasures = 0;
  const unsigned int measurements = sizeof(heightMeasurements) / sizeof(unsigned int);
  for(unsigned int i = 0; i < measurements; i++) heightMeasurements[i] = 0;
}

void setHeight(unsigned long h) {
  if(h < 70) { // this number needs to be believeable
    const unsigned int measurements = sizeof(heightMeasurements) / sizeof(unsigned int);
    heightMeasurements[heightMeasures % measurements] = h;
    heightMeasures++;
    DEBUG(String("Height is ")+String(h)+String(" and number of measurements is ")+String(heightMeasures));
  }
}

float heightAverage(void) {
  float averageHeight = 0.0;
  const unsigned int measurements = sizeof(heightMeasurements) / sizeof(unsigned int);
  float dailyMeasurements = (float) heightMeasures < measurements ? heightMeasures : measurements;
  for(unsigned int i = 0; i < dailyMeasurements; i++) averageHeight += heightMeasurements[i];
  DEBUG(String("Average height is ")+String(averageHeight));
  return averageHeight / dailyMeasurements;
}

void flowerCheck(void) {
    float averageHeight = heightAverage(); 
    if(heightMeasures >= 5) { // If we don't have enough so data don't do anything
      if(averageHeight >= FLOWER_AT_HEIGHT && c.getLightOnTime() != LIGHT_ON_TIME_FLOWER) {
        c.setLightOnTime(LIGHT_ON_TIME_FLOWER);
        c.setLightStartTime(LIGHT_START_TIME_FLOWER);
        startFlowering();
      } else if(averageHeight < FLOWER_AT_HEIGHT && c.getLightOnTime() != LIGHT_ON_TIME_VEG) {
        c.setLightOnTime(LIGHT_ON_TIME_VEG);
        c.setLightStartTime(LIGHT_START_TIME_VEG);
      }
      resetHeightMeasurements();
    }
}

void startFlowering(void) {
  static unsigned int start;
  if(start < 12) {
    start++;
    Alarm.timerOnce(300, startFlowering);
    e.createEvent("",START_FLOWERING);
  }
}

const unsigned long getGrowMode(void) {
  if(c.getLightOnTime() == LIGHT_ON_TIME_FLOWER) {
    return 1;
  } else if(c.getLightOnTime() == LIGHT_ON_TIME_VEG) {
    return 0;
  } else {
    return 3;
  }
}

/* Fan Functions */

void setTemp(const unsigned long h) {
  DEBUG(String("Temp is ")+String(h));
  if(h >= DESIRED_AIR_TEMP) {
    DEBUG("Temp is too high. Turning on fan.");
    array.turnOnTwo(); // turn on fan
    e.createEvent("1",SET_FAN_ON); // Relay this out
  } else if(array.isOnTwo()) {
    DEBUG("Temp is ok now. Turning off fan.");
    array.turnOffTwo(); // turn off fan
    e.createEvent("0",SET_FAN_ON); // Relay this out
  }
  lastTempNotification = now();
}

void getTemp(void) {
  DEBUG(String("Time since last notification ")+String(now() - lastTempNotification));
  if(now() - lastTempNotification > 300 && !array.isOnTwo()) {
    // We haven't heard from the temp sensor in 5 minutes
    // and the fan is off so turn on the fan
    array.turnOnTwo();
    e.createEvent("1",SET_FAN_ON); // Relay this out
    DEBUG("We haven't heard from the temp sensor in over 5 minutes!");
  }
  e.createEvent("",GET_AIR_TEMP);
}

/* Status Functions */

void getHeight(void) {
  e.createEvent("",GET_DISTANCE);
}

void getHumidity(void) {
  e.createEvent("",GET_HUMIDITY);
}

const unsigned long fanIsOn(void) {
  return array.isOnTwo() ? 1 : 0;
}

const unsigned long pumpIsOn(void) {
  return array.isOnThree() ? 1 : 0;
}

const unsigned long foggerIsOn(void) {
  return array.isOnFour() ? 1 : 0;
}

const unsigned long lightIsOn(void) {
  return array.isOnOne() ? 1 : 0;
}

const unsigned long getMoisture(void) {
  return moisture.getReading();
}

const unsigned long getTime(void) {
  return now();
}
