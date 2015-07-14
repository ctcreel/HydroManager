#ifndef Logger_h
#define Logger_h

#include <Arduino.h>

class logger {
  private:
    boolean sdPresent;
    logger(const logger &s) = delete;

  public:
    logger(byte p1, byte p2, byte p3, byte p4);
    boolean const logMessage(const char *fileName, const char *message);
    boolean const sdIsPresent(void);
};

#endif
