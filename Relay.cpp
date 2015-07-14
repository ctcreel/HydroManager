#include "Relay.h"

relay::relay(byte rp) {
  relayPin = rp;
  relayOn = false;
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);        // Prevents relays from starting up engaged
}

boolean relay::turnOn(void) {
  if(relayOn == false) {
    digitalWrite(relayPin, LOW);   // energizes the relay
    relayOn = true;
    return true;
  } else {
    return false;
  }
}

boolean relay::turnOff(void) {
  if(relayOn == true) {
    digitalWrite(relayPin, HIGH);   // energizes the relay
    relayOn = false;
    return true;
  } else {
    return false;
  }
}

boolean const relay::isOn(void) {
  return relayOn;
}
