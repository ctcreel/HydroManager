#ifndef manageROM_h
#define manageROM_h

class manageROM {
  private:
    struct layout {
      unsigned long initialized;
      unsigned long desiredHumidity;
      unsigned long pumpOnTime;
      unsigned long lightOnTime;
      unsigned long lightStartTime;
      unsigned long coolDown;
      unsigned long checkInterval;
    } static configuration;
    
    static void writeConfiguration(void);
    static void readConfiguration(void);
    
    manageROM(const manageROM &s) = delete;

    // virtual bool handler(const char *payload, const unsigned int deviceTypeID, const unsigned int deviceID);
    
  public:
    manageROM(
      unsigned long desiredHumidity,
      unsigned long pumpOnTime,
      unsigned long lightOnTime,
      unsigned long lightStartTime,
      unsigned long coolDown,
      unsigned long checkInterval);
    manageROM(void);
    
    static void reset(void);    
    static const unsigned long getDesiredHumidity(void);
    static const unsigned long getPumpOnTime(void);
    static const unsigned long getLightStartTime(void);
    static const unsigned long getLightOnTime(void);
    static const unsigned long getCoolDownTime(void);
    static const unsigned long getCheckInterval(void);
    static void setDesiredHumidity(unsigned long);
    static void setPumpOnTime(unsigned long);
    static void setLightStartTime(unsigned long);
    static void setLightOnTime(unsigned long);
    static void setCoolDownTime(unsigned long);
    static unsigned long setCheckInterval(unsigned long);
};

#endif
