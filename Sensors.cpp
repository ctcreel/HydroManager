#include "DHT.h"
#include "Sensors.h"
#include <TimeAlarms.h>

#define DHTTYPE DHT22   // DHT 22  (AM2302)

void pinPoweredSensor::turnPowerOn(void) {
  analogWrite(powerPin, HIGH);
}

void pinPoweredSensor::turnPowerOff(void) {
  analogWrite(powerPin, LOW);
}

byte pinPoweredSensor::getDataPin(void) {
  return dataPin;
}

pinPoweredSensor::pinPoweredSensor(byte d, byte p, unsigned long pUI) {
  dataPin = d;
  powerPin = p;
  powerUpInterval = pUI;
}

const float pinPoweredSensor::getReading(void) {  
  turnPowerOn();
  Alarm.delay(powerUpInterval);
  float r = fetchValue();
  if(powerUpInterval != 0) {
    turnPowerOff();
  }
  return r;
}

sensorAnalog::sensorAnalog (
  byte dataPin, 
  byte powerPin, 
  unsigned long powerUpInterval,
  float rH, // max possible reading
  float rL // min possible reading
): pinPoweredSensor(dataPin, powerPin, powerUpInterval) {
  highVoltage = 1024 * (3.0/5.0); // B - output from sensor is 3 volts range for arduino is 5
  readingHigh = rH;
  readingLow = rL;
};

float sensorAnalog::fetchValue(void) {
  float d = analogRead(getDataPin());
  return 1 + (d*(readingHigh-readingLow))/(highVoltage);
}

sensorDigital::sensorDigital(byte dataPin, byte powerPin, unsigned long powerUpInterval)  : 
  pinPoweredSensor(dataPin, powerPin, powerUpInterval) {};

float sensorDigital::fetchValue(void) {
  return digitalRead(getDataPin());
}

sensorDT22::sensorDT22(byte dataPin, byte powerPin, unsigned long powerUpInterval) : 
  pinPoweredSensor(dataPin, powerPin, powerUpInterval) {
    dht = new DHT(getDataPin(), DHT22);
}

sensorHumidity::sensorHumidity(byte dataPin, byte powerPin, unsigned long powerUpInterval) : 
  sensorDT22(dataPin, powerPin,powerUpInterval) {};

float sensorHumidity::fetchValue(void) {
  float h = dht->readHumidity();
}

sensorTemp::sensorTemp(byte dataPin, byte powerPin, unsigned long powerUpInterval) : 
  sensorDT22(dataPin, powerPin,powerUpInterval) {};

float sensorTemp::fetchValue(void) {
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht->readTemperature(true);
}
