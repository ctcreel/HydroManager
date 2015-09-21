#include "Relay.h"

relay::relay(byte rp) {
  relayPin = rp;
  relayOn = false;
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);        // Prevents relays from starting up engaged
}

void relay::turnOn(void) {
  if(relayOn == false) {
    digitalWrite(relayPin, LOW);   // energizes the relay
    relayOn = true;
  }
}

void relay::turnOff(void) {
  if(relayOn == true) {
    digitalWrite(relayPin, HIGH);   // energizes the relay
    relayOn = false;
  }
}

boolean const relay::isOn(void) {
  return relayOn;
}
