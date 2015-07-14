#ifndef pump_h
#define pump_h

#include "Relay.h"

class pump {
    private:
        static relay *deviceRelay;
        
        static void turnOff(void);
        pump(const pump &p) = delete;

    public:
        explicit pump(byte pin);
        ~pump(void);
        static void turnOn(unsigned long timeOn);
        static const boolean isOn(void);
};

#endif
