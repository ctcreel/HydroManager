#include "pump.h"
#include "DEBUG.h"
#include <TimeAlarms.h>

relay *pump::deviceRelay;

pump::pump(byte pin) {
    deviceRelay = new relay(pin);
}

pump::~pump(void) {
  delete deviceRelay;
}

void pump::turnOff(void) {
    deviceRelay->turnOff();
    DEBUG("Turning pump off");
}

void pump::turnOn(unsigned long timeOn) {
    if(deviceRelay->turnOn()){
        Alarm.timerOnce(timeOn, turnOff);
        DEBUG("pump turned on and schedule to turn off in " + String(timeOn) + " seconds");
    }
}

boolean const pump::isOn(void) {
  return deviceRelay->isOn();
};

