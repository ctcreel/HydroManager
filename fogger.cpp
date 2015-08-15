#include "fogger.h"
#include "DEBUG.h"
#include <TimeAlarms.h>

relay *fogger::deviceRelay;

fogger::fogger(byte pin) {
    deviceRelay = new relay(pin);
}

fogger::~fogger(void) {
  delete deviceRelay;
}

void fogger::turnOff(void) {
    deviceRelay->turnOff();
    DEBUG("Turning fogger off");
}

void fogger::turnOn(unsigned long timeOn) {
    if(deviceRelay->turnOn()){
        Alarm.timerOnce(timeOn, turnOff);
        DEBUG("fogger turned on and schedule to turn off in " + String(timeOn) + " seconds");
    }
}

const boolean fogger::isOn(void) {
  return deviceRelay->isOn();
};

