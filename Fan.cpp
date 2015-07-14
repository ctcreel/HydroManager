#include "fan.h"
#include "DEBUG.h"
#include <TimeAlarms.h>

relay *fan::deviceRelay;

fan::fan(byte pin) {
    deviceRelay = new relay(pin);
}

fan::~fan(void) {
  delete deviceRelay;
}

void fan::turnOff(void) {
    deviceRelay->turnOff();
    DEBUG("Turning fan off");
}

void fan::turnOn(unsigned long timeOn) {
    if(deviceRelay->turnOn()){
        Alarm.timerOnce(timeOn, turnOff);
        DEBUG("fan turned on and schedule to turn off in " + String(timeOn) + " seconds");
    }
}

const boolean fan::isOn(void) {
  return deviceRelay->isOn();
};

