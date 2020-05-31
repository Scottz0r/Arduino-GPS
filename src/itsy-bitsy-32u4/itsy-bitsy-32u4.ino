/*
    GPS with LCD Display Arduino Project.

    Author: Scott Clewell

    Required Libraries:
    - Adafruit GFX: https://github.com/adafruit/Adafruit-GFX-Library
    - Adafruit ST7735: https://github.com/adafruit/Adafruit-ST7735-Library
    - MicroGps: https://github.com/Scottz0r/MicroGps
*/
#include <arduino.h>
#include <MicroGps.h>
#include <MicroGpsFormat.h>

#include "prj_pins.h"
#include "prj_display.h"
#include "mtk3339_startup.h"

using namespace scottz0r::gps;

auto gps = MicroGps();

// Extern declared in prj_display.
TftDisplay tft_display = TftDisplay(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

void setup()
{
    // Configure pins
    pinMode(PIN_GPS_ENABLE, OUTPUT);

    tft_display.init();
    tft_display.start_print_mode();

    mtk3339::ReturnCode init_rc = mtk3339::init();
    switch(init_rc)
    {
    case mtk3339::ReturnCode::StartupError:
        tft_display.println(F("GPS startup failed!"));
        break;
    case mtk3339::ReturnCode::ConfigError:
    tft_display.println(F("GPS configuration failed!"));
        break;
    }

    // Do something on failure?

    tft_display.display_wait();
}

void loop()
{
    // Collect NMEA messages from GPS serial device.
    while(GpsSerial.available())
    {
        char c = GpsSerial.read();
        if(gps.process(c))
        {
            if(gps.good() && gps.message_type() == MicroGps::MessageType::GPGGA)
            {
                if(gps.position_data().fix_quality == 0)
                {
                    tft_display.display_wait();
                }
                else
                {
                    tft_display.display_position(gps.position_data());
                }
            }
            else if(gps.bad())
            {
                tft_display.display_fail();
            }

            // Stop looping if a full message has been parsed. To allow other items to process.
            break;
        }
    }

    tft_display.process();
}
