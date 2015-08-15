#ifndef fogger_h
#define fogger_h

#include "Relay.h"

class fogger {
    private:
        static relay *deviceRelay;
        
        static void turnOff(void);
        fogger(const fogger &p) = delete;

    public:
        explicit fogger(byte pin);
        ~fogger(void);
        static void turnOn(unsigned long timeOn);
        const static boolean isOn(void);
};

#endif
