#include "light.h"
#include "DEBUG.h"
#include <TimeAlarms.h>

relay *light::deviceRelay;

light::light(byte pin) {
    deviceRelay = new relay(pin);
}

light::~light(void) {
  delete deviceRelay;
}

void light::turnOff(void) {
    deviceRelay->turnOff();
    DEBUG("Turning light off");
}

void light::turnOn(unsigned long timeOn) {
    if(deviceRelay->turnOn()){
        Alarm.timerOnce(timeOn, turnOff);
        DEBUG("light turned on and schedule to turn off in " + String(timeOn) + " seconds");
    }
}

const boolean light::isOn(void) {
  return deviceRelay->isOn();
};

