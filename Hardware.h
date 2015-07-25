#ifndef Hardware_h
#define Hardware_h

/**************************/
/* Devices - digital pins */
/**************************/
#define XBEE_TX 53
#define XBEE_RX 52
/**************************/
#define SD_ONE 10
#define SD_TWO 11
#define SD_THREE 12
#define SD_FOUR 13

// 4or10 ->53, 11->51, 12 ->50, and 13 ->52.
/**************************/
#define PUMP_PIN 45
#define FAN_PIN 47
#define LIGHT_PIN 49
#define TEMP_POWER 52
#define MOISTURE_POWER 53
#define WATER_LEVEL_POWER 22

/*************************/
/* Devices - analog pins */
/*************************/
#define WATER_LEVEL_PIN 13
#define TEMP_PIN 14
#define MOISTURE_PIN 15

/********************/
/* Arduino Settings */
/********************/
#define MAX_OUTPUT_VOLTAGE 5
#define MAX_DIGITAL_OUTPUT 1024

#endif
