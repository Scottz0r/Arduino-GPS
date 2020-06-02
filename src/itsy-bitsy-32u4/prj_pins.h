#ifndef _PRJ_PINS_INCLUDE_GUARD
#define _PRJ_PINS_INCLUDE_GUARD

#include <Arduino.h>

// GPS Serial Device
#define GpsSerial Serial1

// GPS Pins
#define PIN_GPS_ENABLE 7

// TFT Display Pins
#define PIN_TFT_CS 10
#define PIN_TFT_DC 9
#define PIN_TFT_RST -1

#define DEBUG_MESSAGES 0

// Watchdog timeout for waiting for GPS messages over serial connection.
#define SERIAL_WATCHDOG_TIMEOUT_MS 10000

// Maximum number of sequential bad NMEA messages before hard erroring.
#define MAX_BAD_NMEA_MESSAGES 3

#endif // _PRJ_PINS_INCLUDE_GUARD
