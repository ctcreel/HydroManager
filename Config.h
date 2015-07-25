#ifndef Config_h
#define Config_h

/********************/
/* Default Settings */
/********************/

#define INITIALIZATION_CODE 0x1A6F
#define DESIRED_HUMIDITY 80
#define PUMP_ON_TIME 15
#define LIGHT_ON_TIME_VEG (18L * 3600L)
#define LIGHT_START_TIME_VEG (3L * 3600L)
#define LIGHT_ON_TIME_FLOWER (12L * 3600L)
#define LIGHT_START_TIME_FLOWER (9L * 3600L)
#define COOL_DOWN 600L
#define CHECK_MOISTURE_INTERVAL 60
#define CHECK_HEIGHT_INTERVAL 3600

/* Environment settings */
#define HEIGHT_STABLE 4
#define TENT_HEIGHT  70.0
#define POT_HEIGHT  15.0
#define LIGHT_DISTANCE 18.0
#define LIGHT_ASSEMBLY  6.0
#define LIGHT_HEIGHT 3.0
#define GROWTH_AFTER_VEG  1.4
#define MEASUREMENTS 100
#define MIN_MEASUREMENTS 5

#endif
