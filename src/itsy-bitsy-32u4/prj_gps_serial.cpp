/*
    Adafruit Isty Bitsy 32u4 Gps Serial implementation.

    Assumes that Serial1 will be initialized before use of this class.
*/
#include <HardwareSerial.h>
#include <Arduino.h>

#include "prj_gps_serial.hpp"

using namespace Scottz0r;

int GpsSerial::available()
{
    return Serial1.available();
}

int GpsSerial::read()
{
    // Also print to serial to see NMEA messages.
    int c = Serial1.read();
    Serial.write(c);
    return c;

    // return Serial1.read();
}
