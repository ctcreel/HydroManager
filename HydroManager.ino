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
unsigned long lastTempAttempt;
unsigned long lastHumidityAttempt;
boolean humidityTooHigh;
boolean tempTooHigh;
boolean tempEmergency;
boolean moistureTooLow;
unsigned long lastMoistureAttempt;


RTC_DS1307 RTC;
time_t syncProvider()     //this does the same thing as RTC_DS1307::get()
{
  return RTC.now().unixtime();
}

void setup () {
 
  
  Wire.begin();
  Serial.begin(BAUD_RATE);
  Serial3.begin(BAUD_RATE); // XBee

  c.reset(); // reset flash settings
  // setTime(18,59,50,1,1,2016);
  RTC.begin();
  setSyncProvider(syncProvider);     //reference our syncProvider function instead of RTC_DS1307::get()
  DEBUG("Current time is - " + String(hour()) + ":" + String(minute()) + " on " + String(month()) + "-" + String(day()) + "-" + String(year()));

  /* Set up remote devices */
  new eventIncoming(&e, setHumidity, SET_HUMIDITY);
  new eventIncoming(&e, setTemp, SET_AIR_TEMP);
  new eventIncoming(&e, setHeight, SET_HEIGHT);

  /* Set up report out */
  new eventOutgoing(&e, getTime, SET_TIME, GET_TIME);
  new eventOutgoing(&e, getGrowMode, SET_GROW_MODE, GET_GROW_MODE);
  new eventOutgoing(&e, getLightOn, SET_LIGHT_ON, GET_LIGHT_ON);
  new eventOutgoing(&e, getAverageHeight, SET_AVERAGE_HEIGHT, GET_AVERAGE_HEIGHT);

  resetHeightMeasurements();
  lastTempAttempt = lastHumidityAttempt = lastMoistureAttempt = now();
  humidityTooHigh = tempTooHigh = moistureTooLow = tempEmergency = false;
  
  /* Set up alarms */
  Alarm.timerRepeat(c.getHeightInterval(), getHeight);
  Alarm.timerRepeat(c.getHumidityInterval(), getHumidity);
  Alarm.timerRepeat(c.getTempInterval(), getTemp);
  Alarm.timerRepeat(c.getMoistureInterval(), checkMoisture);
  Alarm.alarmRepeat(c.getLightStartTime(),scheduleLight);
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
  const long timeInDay = 24L* 60L*60L;
  const long current = (second() + (((long) minute())*60L) + (((long) hour())*3600L))+1; // +1 to avoid edge case
  const long startTime = c.getLightStartTime();
  const long runTime = c.getLightOnTime();
  const long endTime = startTime + runTime;
  const long nightTime = timeInDay - runTime;
  const long timeLeft = (((endTime - current) + nightTime)) % timeInDay;
  const long clockOnTime = timeInDay- (timeInDay - timeLeft) - nightTime;

  DEBUG("Setting up clock");
  DEBUG(current);
  DEBUG(endTime);
  DEBUG(timeInDay);
  DEBUG(nightTime);
  DEBUG(timeLeft);
  DEBUG(clockOnTime);

  if(clockOnTime > 0) {
    DEBUG("Woke up during the day");
    array.turnOnOne(clockOnTime+1); // add that second back!
  } else {
    DEBUG("Woke up during the night");
    array.turnOffOne();
  }
}

/* Fan Control */
void setTemp(const unsigned long h) {
  DEBUG(String("Temp is ")+String(h));

  if(h >= c.getMaxAirTemp()) {
    temperatureEmergency();
  } else if(h >= c.getDesiredAirTemp()) {
    DEBUG("Temp is too high but no danger. Turning on fan.");
    tempTooHigh = true;
    array.turnOnTwo(); // turn on fan
    getHumidity(); // start checking humidity
    e.createEvent("1",SET_FAN_ON); // Relay this out
  } else {
      tempTooHigh = false;
      lastTempAttempt = now();
      if(humidityTooHigh == false) {
        array.turnOffTwo(); // turn off fan
        getHumidity();
        e.createEvent("0",SET_FAN_ON); // Relay this out
      }
      if(tempEmergency == true) {
        tempEmergency = false;
        scheduleLight();
      }
    }
}

void temperatureEmergency(void) {
  DEBUG("Temp is danegrously high, turning off light!");
  if(!tempEmergency) {
    array.turnOffOne();
    tempEmergency = true;
  }
  e.createEvent("1",TEMP_EMERGENCY);
}

void getTemp(void) {
  e.createEvent("0",GET_AIR_TEMP);
  if(now() - lastTempAttempt >= GIVE_UP_ON_LIGHT_AFTER) {
    temperatureEmergency();
  }
}

/* Fogger Control */

void humidityLowCheck(const unsigned long h) {
  if(h < c.getDesiredHumidity() && now() - lastHumidityAttempt < GIVE_UP_ON_FOGGER_AFTER) {
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
    if(h >= c.getDesiredHumidity()) {
      // were able to control the humidity
      lastHumidityAttempt = now();
    } else {
      e.createEvent("1",HUMIDITY_EMERGENCY);
    }
  }
}

void humidityHighCheck(const unsigned long h) {
  if(h >= c.getMaxHumidity()) {
    humidityTooHigh = true;
    array.turnOffFour(); // turn off fogger if it is on
    e.createEvent("1",SET_FAN_ON);
    array.turnOnTwo();
    Alarm.timerOnce(10, getHumidity);
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
  if(m < c.getDesiredMoisture() && now() - lastMoistureAttempt < GIVE_UP_ON_MOISTURE_AFTER) {
    array.turnOnThree(c.getPumpOnTime());
    moistureTooLow = true;
    e.createEvent("1",SET_PUMP_ON);
  } else {
    if(m >= c.getDesiredMoisture()) {
      if(moistureTooLow) {
        e.createEvent("0",SET_PUMP_ON);
        moistureTooLow = false;
      }
      lastMoistureAttempt = now();
    }
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
    e.createEvent("0",START_FLOWERING);
  }
}

/* Status Functions */

const unsigned long getAverageHeight(void) {
  return heightAverage();
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

void getHeight(void) {
  e.createEvent("0",GET_DISTANCE);
}

void getHumidity(void) {
  e.createEvent("0",GET_HUMIDITY);
}

const unsigned long  getLightOn(void) {
  return array.isOnOne();
}

const unsigned long getTime(void) {
  return now();
}

