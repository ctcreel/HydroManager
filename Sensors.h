#ifndef Sensor_h
#define Sensor_h

#include <Arduino.h>

class sensor {
  
  private:
    byte data;
    byte power;
    unsigned int powerUpInterval;
    float highVoltage;
    float readingHigh;
    float readingLow;
    
    sensor(const sensor &s) = delete;
    
  public:
  
    sensor(
      byte data, // data pin
      byte power, // power pin
      unsigned int powerUpInterval, // how long it takes to power up the device
      float highVoltage, // device output voltage
      float readingHigh, // max possible reading
      float readingLow // min possible reading
    );
    const float getReading(void);
};

#endif

