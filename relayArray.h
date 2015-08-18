#ifndef relayArray_h
#define relayArray_h

#include "Relay.h"

class relayArray {
    private:
        static relay *relayOne;
        static relay *relayTwo;
        static relay *relayThree;
        static relay *relayFour;
        
        static void turnOffOne(void);
        static void turnOffTwo(void);
        static void turnOffThree(void);
        static void turnOffFour(void);
        
        relayArray(const relayArray &p) = delete;

    public:
        explicit relayArray(byte pinOne, byte pinTwo, byte pinThree, byte pinFour);
        ~relayArray(void);
        static void turnOnOne(unsigned long timeOn);
        static void turnOnTwo(unsigned long timeOn);
        static void turnOnThree(unsigned long timeOn);
        static void turnOnFour(unsigned long timeOn);
        const static boolean isOnOne(void);
        const static boolean isOnTwo(void);
        const static boolean isOnThree(void);
        const static boolean isOnFour(void);
};

#endif
