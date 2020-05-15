#include <arduino.h>

#include "Adafruit_ST7789.h"

#include "prj_gps_serial.hpp"
#include "buffered_gps.hpp"
#include "nmea_parser.hpp"
#include "gps_format.hpp"

using namespace Scottz0r;

#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST -1

#define DISPLAY_DELAY_MS 500

void display_gps_posn();
void display_wait();
void display_fail();
void init_gps_serial();

auto gps = BufferedGps();
GpsPosition gps_posn;

#define STATE_POSITION 1
#define STATE_WAIT 2
#define STATE_FAIL 3
int state = -1;

unsigned long last_posn;

// TODO: This should probably get moved at some point.
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
    Serial.begin(115200);
    init_gps_serial();

    gps.begin();

    tft.init(135, 240);
    tft.setRotation(0);
    tft.fillScreen(0x0000);

    tft.setRotation(1);

}

void init_gps_serial()
{
    // Change the baud rate to 57,600. This allows less time between reading all messages and 
    // writing to the display.
    // Serial1.begin(9600);
    // delay(100);
    // Serial1.println(F("$PMTK251,57600*2C")); // 57,600 baud.
    // delay(100);
    // Serial1.end();
    // delay(100);
    // Serial1.begin(57600);
    // delay(100);

    // Above is super unstable. Use 9600 baud and get GGA only.
    Serial1.begin(9600);

    // Set Polling rate on GPS to 5 seconds.
    Serial1.println(F("$PMTK220,5000*1B"));
    // Serial1.println(F("$PMTK314,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28")); // This is GGA and GSV
    // GSV is a bit much for a tiny screen and embedded 16MHz device, so sticking with Position only.
    Serial1.println(F("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29")); // GGA only.
}

void loop()
{
    gps.process();

    // Super messy, but prototype code. Fix later.
    // Should probably always run this stuff, but leave displaying stuff to specific "screens".
    if(gps.sentence_available())
    {
        if(!NmeaParser::checksum(gps.sentence(), gps.sentence_size()))
        {
            // display_fail();
            Serial.print("Checksum Fail: ");
            Serial.print(gps.sentence());
        }
        else if(strncmp(gps.sentence(), "$GPGGA", sizeof("$GPGGA") - 1) == 0)
        {
            if(!NmeaParser::parse_gga(gps.sentence(), gps.sentence_size(), gps_posn))
            {
                display_fail();
                Serial.print("Parse Fail: ");
                Serial.print(gps.sentence());
            }
            else if(gps_posn.has_fix)
            {
                // TODO: This is too slow, so GPS stuff is dropped while display is rendering.
                // There needs to be a way to do this without blocking serial.
                // display_gps_posn();
                last_posn = millis();
            }
            else
            {
                display_wait();
            }
        }
        
        gps.clear_sentence();
    }

    // Wait a few milliseconds to update the display. This is done because the serial spits a lot
    // of data at once, and writing the display is super slow.
    if(last_posn)
    {
        if(millis() - last_posn >= DISPLAY_DELAY_MS)
        {
            Serial.print("millis() - last_posn = ");
            Serial.print(millis() - last_posn);
            Serial.println(". Updating display.");
            display_gps_posn();
            last_posn = 0;
        }
    }
}

void display_gps_posn()
{
    if(state != STATE_POSITION)
    {
        tft.fillScreen(0x0000);
        state = STATE_POSITION;
    }

    tft.setCursor(0, 0);
    tft.setTextColor(0xFFFF, 0x0000);
    tft.setTextSize(2);

    char fmt_buffer[32];
    GpsFormat::format_lat_ddmm(gps_posn.latitude, fmt_buffer, sizeof(fmt_buffer));
    tft.println(fmt_buffer);
    //Serial.println(fmt_buffer);

    GpsFormat::format_lon_ddmm(gps_posn.longitude, fmt_buffer, sizeof(fmt_buffer));
    tft.println(fmt_buffer);
    //Serial.println(fmt_buffer);
}

void display_wait()
{
    if(state != STATE_WAIT)
    {
        tft.fillScreen(0x0000);
        state = STATE_WAIT;
    }

    tft.setCursor(0, 0);
    tft.setTextColor(0xFFFF, 0x0000);
    tft.setTextSize(2);
    tft.print("Waiting for fix...");
}

void display_fail()
{
    if(state != STATE_WAIT)
    {
        tft.fillScreen(0x0000);
        state = STATE_WAIT;
    }

    tft.setCursor(0, 0);
    tft.setTextColor(0xFFFF, 0x0000);
    tft.setTextSize(2);
    tft.print("GPS Fail!");
}
