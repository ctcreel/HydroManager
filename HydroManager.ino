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

unsigned int heightMeasurements[10];
unsigned int heightMeasures;
unsigned long lastTempNotification;
boolean humidityTooHigh;
boolean tempTooHigh;

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
  Serial3.begin(BAUD_RATE); // XBee

  /* Set up clock */ 
  RTC.begin();
  setSyncProvider(syncProvider);     //reference our syncProvider function instead of RTC_DS1307::get()
  DEBUG("Current time is - " + String(hour()) + ":" + String(minute()) + " on " + String(month()) + "-" + String(day()) + "-" + String(year()));

  /* Set up remote devices */
  new eventIncoming(&e, setHumidity, SET_HUMIDITY);
  new eventIncoming(&e, setTemp, SET_AIR_TEMP);
  new eventIncoming(&e, setHeight, SET_HEIGHT);

  DEBUG("Light time on - " + String(c.getLightOnTime()));
  DEBUG("Light start time - " + String(c.getLightStartTime()));
  DEBUG("Grow mode is - " + String(getGrowMode()));
  DEBUG("Flower at height is - "+ String(FLOWER_AT_HEIGHT));

  resetHeightMeasurements();
  lastTempNotification = now();
  humidityTooHigh = false;
  tempTooHigh = false;
  
  /* Set up alarms */
  Alarm.timerRepeat(c.getHeightInterval(), getHeight);
  Alarm.timerRepeat(c.getHumidityInterval(), getHumidity);
  Alarm.timerRepeat(c.getTempInterval(), getTemp);
  Alarm.timerRepeat(c.getMoistureInterval(), checkMoisture);
  Alarm.alarmRepeat(24,00,00,scheduleLight);
  Alarm.alarmRepeat(24,00,00,flowerCheck);

  /* Let's get started */
  getHeight();
  getHumidity();
  getTemp();
  checkMoisture();
  scheduleLight();
 }

void loop()
{  
  e.check(0);
}

/* Light Control */

void scheduleLight(void) {
  const unsigned long current = (second() + (((unsigned long) minute())*60) + (((unsigned long) hour())*3600));
  const unsigned long startTime = c.getLightStartTime();
  const unsigned long endTime = startTime + c.getLightOnTime();
  if(current >= 0 && current < startTime) {
    Alarm.timerOnce(startTime, scheduleLight);
  } else if(current >= startTime && current < endTime) {
    array.turnOnOne(endTime - current); // Turn on light
  }
}

/* Fan Control */

void setTemp(const unsigned long h) {
  DEBUG(String("Temp is ")+String(h));
  if(h >= c.getDesiredAirTemp()) {
    DEBUG("Temp is too high. Turning on fan.");
    tempTooHigh = true;
    array.turnOnTwo(); // turn on fan
    getHumidity(); // start checking humidity
    e.createEvent("1",SET_FAN_ON); // Relay this out
  } else {
    tempTooHigh = false;
    if(humidityTooHigh == false) {
      array.turnOffTwo(); // turn off fan
      getHumidity();
      e.createEvent("0",SET_FAN_ON); // Relay this out
      DEBUG("Temp is ok now. Turning off fan.");
    }
  }
  lastTempNotification = now();
}

void getTemp(void) {
  DEBUG(String("Time since last notification ")+String(now() - lastTempNotification));
  if(now() - lastTempNotification > TURN_FAN_ON_AFTER && !array.isOnTwo()) {
    // We haven't heard from the temp sensor in 60 seconds
    // and the fan is off so turn on the fan
    array.turnOnTwo();
    e.createEvent("1",SET_FAN_ON); // Relay this out
    DEBUG("We haven't heard from the temp sensor in over 60 seconds!");
  }
  e.createEvent("",GET_AIR_TEMP);
}

/* Fogger Control */

void humidityLowCheck(const unsigned long h) {
  if(h < c.getDesiredHumidity()) {
    humidityTooHigh = false;
    e.createEvent("1",SET_FOGGER_ON);
    if(tempTooHigh == false) {
      // the humidity is just low so add a little water
      array.turnOnFour(FOGGER_ON_TIME);
    } else {
      // the temp is too high so just turn on the fogger
      array.turnOnFour();
    }
  } else {
    array.turnOffFour(); // turn off fogger
    e.createEvent("0",SET_FOGGER_ON);
  }
}

void humidityHighCheck(const unsigned long h) {
  if(h >= c.getMaxHumidity()) {
    humidityTooHigh = true;
    array.turnOffFour(); // turn off fogger if it is on
    e.createEvent("1",SET_FAN_ON);
    if(array.isOnOne()) {
      // if the light is on then just turn the fan on for a moment
      array.turnOnTwo(FAN_ON_FOR_HUMIDITY);
    } else {
      // otherwise just turn it on full blast
      array.turnOnTwo();
    }
  } else {
    humidityTooHigh = false;
    if(array.isOnTwo() && tempTooHigh == false) {
      array.turnOffTwo(); // turn off fan
      e.createEvent("0",SET_FAN_ON);
    }
  }
}

void setHumidity(const unsigned long h) {
  DEBUG(String("Humidity is ")+String(h));
  DEBUG(String("Desired humidity is ")+String(c.getDesiredHumidity()));
  humidityLowCheck(h);
  humidityHighCheck(h);
}

/* Moisture Control */

void checkMoisture(void) {
  unsigned long m = moisture.getReading();
  DEBUG(String("Moisture is ")+String(m));
  DEBUG(String("Desired moisture is ")+c.getDesiredMoisture());
  if(m < c.getDesiredMoisture()) {
    array.turnOnThree(c.getPumpOnTime());
  }
  e.createEvent(m,SET_MOISTURE);
}

/* Flowering Control */

void resetHeightMeasurements(void) {
  heightMeasures = 0;
  const unsigned int measurements = sizeof(heightMeasurements) / sizeof(unsigned int);
  for(unsigned int i = 0; i < measurements; i++) heightMeasurements[i] = 0;
}

void setHeight(unsigned long h) {
  if(h < TENT_HEIGHT) { // this number needs to be believeable
    const unsigned int measurements = sizeof(heightMeasurements) / sizeof(unsigned int);
    heightMeasurements[heightMeasures % measurements] = h;
    heightMeasures++;
    float averageHeight = heightAverage();
    e.createEvent((long) averageHeight, SET_AVERAGE_HEIGHT);
    DEBUG(String("Average height is ")+String(averageHeight)+String(" and number of measurements is ")+String(heightMeasures));
  }
}

float heightAverage(void) {
  float averageHeight = 0.0;
  const unsigned int measurements = sizeof(heightMeasurements) / sizeof(unsigned int);
  float dailyMeasurements = (float) heightMeasures < measurements ? heightMeasures : measurements;
  for(unsigned int i = 0; i < dailyMeasurements; i++) averageHeight += heightMeasurements[i];
  averageHeight = averageHeight / dailyMeasurements;
  return averageHeight;
}

void setGrowMode(const unsigned int b) {
  if(b == 1) {
    c.setLightOnTime(LIGHT_ON_TIME_FLOWER);
    c.setLightStartTime(LIGHT_START_TIME_FLOWER);
    c.setDesiredHumidity(DESIRED_HUMIDITY_FLOWER);
    startFlowering();
  } else {
    c.setLightOnTime(LIGHT_ON_TIME_VEG);
    c.setLightStartTime(LIGHT_START_TIME_VEG);
    c.setDesiredHumidity(DESIRED_HUMIDITY_VEG);
  }
}

void flowerCheck(void) {
    float averageHeight = heightAverage(); 
    if(heightMeasures >= 5) { // If we don't have enough so data don't do anything
      if(averageHeight >= FLOWER_AT_HEIGHT && c.getLightOnTime() != LIGHT_ON_TIME_FLOWER) {
        setGrowMode(1);
      } else if(averageHeight < FLOWER_AT_HEIGHT && c.getLightOnTime() != LIGHT_ON_TIME_VEG) {
        setGrowMode(0);
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

/* Status Functions */

void getHeight(void) {
  e.createEvent("",GET_DISTANCE);
}

void getHumidity(void) {
  e.createEvent("",GET_HUMIDITY);
}
