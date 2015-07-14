#ifndef light_h
#define light_h

#include "Relay.h"

class light {
    private:
        static relay *deviceRelay;
        
        static void turnOff(void);
        light(const light &p) = delete;

    public:
        explicit light(byte pin);
        ~light(void);
        static void turnOn(unsigned long timeOn);
        const static boolean isOn(void);
};

#endif
