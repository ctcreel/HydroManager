#ifndef fan_h
#define fan_h

#include "Relay.h"

class fan {
    private:
        static relay *deviceRelay;
        
        static void turnOff(void);
        fan(const fan &p) = delete;

    public:
        explicit fan(byte pin);
        ~fan(void);
        static void turnOn(unsigned long timeOn);
        const static boolean isOn(void);
};

#endif
