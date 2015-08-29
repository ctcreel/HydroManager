#ifndef Logger_h
#define Logger_h

#include <Arduino.h>

class logger {
  private:
    char message[20];
    char stamp [20];
    boolean sdPresent;
    void updateStamp(const boolean stamped);
    logger(const logger &s) = delete;

  public:
    logger(byte p1, byte p2, byte p3, byte p4);
    boolean const logMessage(const char *fileName, const char *message, const boolean stamped = true);
    boolean const logValue(const char *fileName, const unsigned long value, const boolean stamped = true);
    boolean const logValue(const char *fileName, const long value, const boolean stamped = true);
    boolean const logValue(const char *fileName, const double value, const boolean stamped = true);
    boolean const logValue(const char *fileName, const char value, const boolean stamped = true);
    boolean const sdIsPresent(void);
};

#endif
