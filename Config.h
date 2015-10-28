#ifndef Config_h
#define Config_h

/********************/
/* Default Settings */
/********************/

#define INITIALIZATION_CODE 0x1A6F
#define DESIRED_MOISTURE 85;
#define DESIRED_HUMIDITY_FLOWER 70
#define DESIRED_HUMIDITY_VEG 50
#define DESIRED_HUMIDITY_SPREAD 10
#define FOGGER_ON_TIME 10
#define FAN_ON_FOR_HUMIDITY 10
#define DESIRED_AIR_TEMP 85
#define MAX_AIR_TEMP 80
#define PUMP_ON_TIME 15
#define LIGHT_ON_TIME_VEG (18L * 3600L)
#define LIGHT_START_TIME_VEG (3L * 3600L)
#define LIGHT_ON_TIME_FLOWER (12L * 3600L)
#define LIGHT_START_TIME_FLOWER (9L * 3600L)
#define CHECK_MOISTURE_INTERVAL 60
#define CHECK_HUMIDITY_INTERVAL 30
#define CHECK_TEMP_INTERVAL 10
#define CHECK_HEIGHT_INTERVAL 900

/* Emergency Settings */
#define TURN_FAN_ON_AFTER 120

/* Environment settings */
#define TENT_HEIGHT  78.0
#define POT_HEIGHT  15.0
#define IDEAL_LIGHT_DISTANCE 18.0
#define SENSOR_TO_TOP_HEIGHT 2.0
#define SENSOR_TO_PLANT_HEIGHT 2.0
#define LIGHT_ASSEMBLY 6.0
#define LIGHT_HEIGHT 3.0
#define GROWTH_AFTER_VEG  1.7
#define FLOWER_AT_HEIGHT ((unsigned long ) ((TENT_HEIGHT - (POT_HEIGHT + IDEAL_LIGHT_DISTANCE + LIGHT_ASSEMBLY))/GROWTH_AFTER_VEG))


#endif
