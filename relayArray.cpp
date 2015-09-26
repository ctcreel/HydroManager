#include "relayArray.h"
#include "DEBUG.h"
#include <TimeAlarms.h>

relay *relayArray::relayOne;
relay *relayArray::relayTwo;
relay *relayArray::relayThree;
relay *relayArray::relayFour;

relayArray::relayArray(byte pinOne,byte pinTwo,byte pinThree,byte pinFour) {
    relayOne = new relay(pinOne);
    relayTwo = new relay(pinTwo);
    relayThree = new relay(pinThree);
    relayFour = new relay(pinFour);
}

relayArray::~relayArray(void) {
  delete relayOne;
  delete relayTwo;
  delete relayThree;
  delete relayFour;
}

void relayArray::turnOffOne(void) {
    relayOne->turnOff();
    DEBUG("Turning relayOne off");
}

void relayArray::turnOffTwo(void) {
    relayTwo->turnOff();
    DEBUG("Turning relayTwo off");
}

void relayArray::turnOffThree(void) {
    relayThree->turnOff();
    DEBUG("Turning relayThree off");
}

void relayArray::turnOffFour(void) {
    relayFour->turnOff();
    DEBUG("Turning relayFour off");
}


void relayArray::turnOnOne(void) {
    relayOne->turnOn();
    DEBUG("Turning relayOne on");
}

void relayArray::turnOnTwo(void) {
    relayTwo->turnOn();
    DEBUG("Turning relayTwo on");
}

void relayArray::turnOnThree(void) {
    relayThree->turnOn();
    DEBUG("Turning relayThree on");
}

void relayArray::turnOnFour(void) {
    relayFour->turnOn();
    DEBUG("Turning relayFour on");
}

void relayArray::turnOnOne(unsigned long timeOn) {
  turnOnOne();
  Alarm.timerOnce(timeOn, turnOffOne);
  DEBUG("relayOne turned on and schedule to turn off in " + String(timeOn) + " seconds");
}

void relayArray::turnOnTwo(unsigned long timeOn) {
  turnOnTwo();
  Alarm.timerOnce(timeOn, turnOffTwo);
  DEBUG("relayTwo turned on and schedule to turn off in " + String(timeOn) + " seconds");
}

void relayArray::turnOnThree(unsigned long timeOn) {
  turnOnThree();
  Alarm.timerOnce(timeOn, turnOffThree);
  DEBUG("relayThree turned on and schedule to turn off in " + String(timeOn) + " seconds");
}

void relayArray::turnOnFour(unsigned long timeOn) {
  turnOnFour();
  Alarm.timerOnce(timeOn, turnOffFour);
  DEBUG("relayFour turned on and schedule to turn off in " + String(timeOn) + " seconds");
}

const boolean relayArray::isOnOne(void) {
  return relayOne->isOn();
};

const boolean relayArray::isOnTwo(void) {
  return relayTwo->isOn();
};

const boolean relayArray::isOnThree(void) {
  return relayThree->isOn();
};

const boolean relayArray::isOnFour(void) {
  return relayFour->isOn();
};

