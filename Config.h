#ifndef Config_h
#define Config_h

/********************/
/* Default Settings */
/********************/

#define INITIALIZATION_CODE 0x1A6F
#define DESIRED_HUMIDITY 80
#define DESIRED_AIR_TEMP 85
#define PUMP_ON_TIME 15
#define LIGHT_ON_TIME_VEG (18L * 3600L)
#define LIGHT_START_TIME_VEG (3L * 3600L)
#define LIGHT_ON_TIME_FLOWER (12L * 3600L)
#define LIGHT_START_TIME_FLOWER (9L * 3600L)
#define COOL_DOWN 600L
#define CHECK_MOISTURE_INTERVAL 60
#define CHECK_HEIGHT_INTERVAL 3600

/* Environment settings */
#define TENT_HEIGHT  77.0
#define POT_HEIGHT  15.0
#define LIGHT_DISTANCE 18.0
#define LIGHT_ASSEMBLY  3.0
#define LIGHT_HEIGHT 3.0
#define GROWTH_AFTER_VEG  1.7
#define FLOWER_AT_HEIGHT ((TENT_HEIGHT - (POT_HEIGHT + LIGHT_DISTANCE + LIGHT_ASSEMBLY + LIGHT_HEIGHT))/GROWTH_AFTER_VEG)


#endif
