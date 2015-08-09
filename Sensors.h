#ifndef SENSOR_STRATEGIES_H
#define SENSOR_STRATEGIES_H

#include "DHT.h"

class sensor {
    
  public:

    const virtual float getReading(void) = 0;
};

class pinPoweredSensor:public sensor {
  private:
    byte dataPin;
    byte powerPin;
    unsigned long powerUpInterval;
    
  protected:
    void turnPowerOn(void);
    void turnPowerOff(void);
    byte getDataPin();
    float virtual fetchValue(void) = 0;
    
  public:
    pinPoweredSensor(byte dataPin, byte powerPin, unsigned long powerUpInterval);
    const float getReading(void);
};

class sensorAnalog :public pinPoweredSensor { 
  private:
    float fetchValue(void);
    float highVoltage;
    float readingHigh;
    float readingLow;

  public:
    sensorAnalog(
      byte dataPin, 
      byte powerPin, 
      unsigned long powerUpInterval,
      float readingHigh, // max possible reading
      float readingLow // min possible reading
    );
};

class sensorDigital :public pinPoweredSensor {
  private:
    float fetchValue(void);

  public:
    sensorDigital(byte dataPin, byte powerPin, unsigned long powerUpInterval);
};

class sensorDT22 : public pinPoweredSensor {
  protected:
    DHT *dht;
    float virtual fetchValue(void) = 0;
  public:
    sensorDT22(byte dataPin, byte powerPin, unsigned long powerUpInterval);
};

class sensorHumidity :public sensorDT22 {
  private:
    float fetchValue(void);

  public:
    sensorHumidity(byte dataPin, byte powerPin, unsigned long powerUpInterval);
};

class sensorTemp :public sensorDT22 {
  private:
    float fetchValue(void);

  public:
    sensorTemp(byte dataPin, byte powerPin, unsigned long powerUpInterval);
};

#endif
