#ifndef Sitter_h
#define Sitter_h

#include "Sensors.h"
#include "Pump.h"
#include "Logger.h"
#include "Light.h"
#include "Fan.h"
#include "manageROM.h"

class sitter {
    private:
    
        static manageROM *configuration;
        static light *lightDevice; // light
        static fan *fanDevice; // light
        static pump *pumpDevice; // pump
        static sensor *moistureSensor; // moisture sensor
        static sensor *tempSensor; // tempreture sensor
        static sensor *levelSensor; // tempreture sensor
        static logger *dataLogger; // SD logger
        static byte lowWaterCount;
        static byte maxLowWaterCount;
        static bool wakeUpAlarmSet;
        
        static void checkAndLog(void);
        static void dailySetup(void);
        static void turnOnLightAndFan(unsigned long timeOn);
        sitter(const sitter &s) = delete;

    public:
          // All times are in seconds
      static const unsigned long sdIsPresent(void);
      static const unsigned long lightIsOn(void);
      static const unsigned long fanIsOn(void);
      static const unsigned long pumpIsOn(void);
      static const unsigned long waterIsLow(void);
      static const unsigned long getSoilTemp(void);
      static const unsigned long getMoisture(void);
      static const unsigned long getTime(void);
      static const unsigned long getWaterLevel(void);
      
      sitter(
          byte maxLowWaterCount,
          manageROM *configuration,
          light *lightDevice,
          fan *fanDevice,
          pump *pumpDevice,
          sensor *level,
          sensor *moisture,
          sensor *temp,
          logger *dataLogger
      );
};

#endif
