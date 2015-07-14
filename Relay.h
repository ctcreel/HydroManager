#ifndef Relay_h
#define Relay_h

#include <Arduino.h>

class relay {
  private:
  
    boolean relayOn;
    byte relayPin;
    relay(const relay &s) = delete;
    
  public:
  
    explicit relay(byte pin);
    boolean turnOn(void);
    boolean turnOff(void);
    boolean const isOn(void);

};

#endif
