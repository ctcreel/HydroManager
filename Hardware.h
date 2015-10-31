#ifndef Hardware_h
#define Hardware_h

/**************************/
/* Devices - digital pins */
/**************************/
#define XBEE_TX 53
#define XBEE_RX 52
#define BAUD_RATE 19200
/**************************/

// 4or10 ->53, 11->51, 12 ->50, and 13 ->52.
/**************************/
#define FOGGER_PIN 43
#define PUMP_PIN 45
#define FAN_PIN 47
#define LIGHT_PIN 49
#define TEMP_POWER 52
#define MOISTURE_POWER 53
#define WATER_LEVEL_POWER 22

/*************************/
/* Devices - analog pins */
/*************************/
#define MOISTURE_PIN 15

/********************/
/* Arduino Settings */
/********************/
#define MAX_OUTPUT_VOLTAGE 5
#define MAX_DIGITAL_OUTPUT 1024

#endif
