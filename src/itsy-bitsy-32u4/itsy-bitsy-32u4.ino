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

#include "prj_display.h"

using namespace scottz0r::gps;

// Pins for small TFT display.
constexpr auto TFT_CS = 10;
constexpr auto TFT_DC = 9;
constexpr auto TFT_RST = -1;

void handle_display(int state);
void display_gps_posn();
void init_gps_serial();
void display_hdop_bar();
void flash_indicator();

auto gps = MicroGps();
auto tft_display = TftDisplay(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
    Serial.begin(115200);
    init_gps_serial();

    tft_display.init();
    tft_display.display_wait();
}

void init_gps_serial()
{
    // Use 9600 baud and get GGA only.
    Serial1.begin(9600);
    delay(100);

    // Set Polling rate on GPS to 5 seconds.
    Serial1.println(F("$PMTK220,5000*1B"));
    delay(100);

    // GSV is a bit much for a tiny screen and embedded 16MHz device, so sticking with Position only.
    Serial1.println(F("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29")); // GGA only.
    delay(100);
}

void loop()
{
    // Collect NMEA messages from GPS serial device.
    while(Serial1.available())
    {
        char c = Serial1.read();
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
