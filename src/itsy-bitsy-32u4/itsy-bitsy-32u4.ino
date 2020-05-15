#include <arduino.h>

#include "Adafruit_ST7789.h"

#include "prj_gps_serial.hpp"
#include "buffered_gps.hpp"
#include "nmea_parser.hpp"
#include "gps_format.hpp"

using namespace Scottz0r;

// Pins for small TFT display.
constexpr auto TFT_CS = 10;
constexpr auto TFT_DC = 9;
constexpr auto TFT_RST = -1;

constexpr auto DISPLAY_DELAY_MS = 250; // How long to wait between a state update request and write.
constexpr auto FLASH_MS = 1000; // Flash delay for active indicator.

void handle_display(int state);
void display_gps_posn();
void display_wait();
void display_fail();
void init_gps_serial();
void display_hdop_bar();
void flash_indicator();

auto gps = BufferedGps();
GpsPosition gps_posn;

constexpr auto STATE_NO_CHANGE = 0;
constexpr auto STATE_POSITION = 1;
constexpr auto STATE_WAIT = 2;
constexpr auto STATE_FAIL = 3;

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

    // Start displaying in state of "waiting"
    display_wait();
}

void init_gps_serial()
{
    // Use 9600 baud and get GGA only.
    Serial1.begin(9600);
    delay(100);

    // Set Polling rate on GPS to 5 seconds.
    Serial1.println(F("$PMTK220,5000*1B"));
    delay(100);
    // Serial1.println(F("$PMTK314,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28")); // This is GGA and GSV

    // GSV is a bit much for a tiny screen and embedded 16MHz device, so sticking with Position only.
    Serial1.println(F("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29")); // GGA only.
    delay(100);
}

void loop()
{
    static unsigned long last_state_change;
    static int next_state;

    gps.process();

    // Super messy, but prototype code. Fix later.
    // Should probably always run this stuff, but leave displaying stuff to specific "screens".
    // TODO: Also need to know if it's been too long since getting data? like > 10 seconds?
    if(gps.sentence_available())
    {
        if(!NmeaParser::checksum(gps.sentence(), gps.sentence_size()))
        {
            next_state = STATE_FAIL;

            Serial.print("Checksum Fail: ");
            Serial.print(gps.sentence());
        }
        else if(strncmp(gps.sentence(), "$GPGGA", sizeof("$GPGGA") - 1) == 0)
        {
            if(!NmeaParser::parse_gga(gps.sentence(), gps.sentence_size(), gps_posn))
            {
                next_state = STATE_FAIL;

                Serial.print("Parse Fail: ");
                Serial.print(gps.sentence());
            }
            else if(gps_posn.has_fix)
            {
                next_state = STATE_POSITION;
            }
            else
            {
                next_state = STATE_WAIT;
            }
        }
        
        gps.clear_sentence();
    }

    // Wait a few milliseconds to update the display. This is done because the serial spits a lot
    // of data at once, and writing the display is super slow.
    if(next_state != STATE_NO_CHANGE)
    {
        if(last_state_change == 0)
        {
            Serial.println("Starting state change timer...");
            last_state_change = millis();
        }

        if(millis() - last_state_change >= DISPLAY_DELAY_MS)
        {
            Serial.println("Now changing state.");
            handle_display(next_state);

            // Reset waiting variables for next state switch.
            next_state = STATE_NO_CHANGE;
            last_state_change = 0;
        }
    }

    // Always process a little flash to indicate system is working.
    flash_indicator();
}

/*
    Handles state changes and calling the right method to update the display.
*/
void handle_display(int state)
{
    static int last_state = -1;

    // Clear entire screen on state change.
    if(last_state != state)
    {
        tft.fillScreen(0x0000);
        state = STATE_POSITION;
    }

    switch(state)
    {
        case STATE_POSITION:
            display_gps_posn();
            break;
        case STATE_WAIT:
            display_wait();
            break;
        case STATE_FAIL:
        default:
            display_fail();
            break;
    }

    last_state = state;
}

/*
    Display all of the GPS position data from GGA sentence.
*/
void display_gps_posn()
{
    tft.setCursor(0, 0);
    tft.setTextColor(0xFFFF, 0x0000);
    tft.setTextSize(2);

    char fmt_buffer[32];
    GpsFormat::format_lat_ddmm(gps_posn.latitude, fmt_buffer, sizeof(fmt_buffer));
    tft.println(fmt_buffer);

    GpsFormat::format_lon_ddmm(gps_posn.longitude, fmt_buffer, sizeof(fmt_buffer));
    tft.println(fmt_buffer);

    // Print other GPS stuff, and pad at end to account for changes magnitude of numbers.
    tft.print(F("Satellites: "));
    tft.print(gps_posn.number_satellites);
    tft.println("    ");

    tft.print(F("Altitude: "));
    tft.print(gps_posn.altitude_msl);
    tft.println("    ");

    tft.setTextSize(1);
    tft.print(F("Geoidal separation: "));
    tft.print(gps_posn.altitude_wgs_84);
    tft.println("    ");

    // Make a pretty "Bar" for HDOP. Should make a real graphic in the future.
    display_hdop_bar();
}

void display_hdop_bar()
{
    tft.setTextSize(2);
    tft.setTextColor(0xFFFF, 0x0000);
    tft.print("HDOP: |");

    // Based off of the dilution, display a number of stars where 6 = best and 1 = worst.
    // Change colors based off the dilution, too.
    int count_stars = 0;
    if(gps_posn.horizontal_dilution <= 1)
    {
        // Ideal
        count_stars = 6;
        tft.setTextColor(0x3F6C, 0x0000);
    }
    else if(gps_posn.horizontal_dilution <= 2)
    {
        // Excellent
        count_stars = 5;
        tft.setTextColor(0x3F6C, 0x0000);
    }
    else if(gps_posn.horizontal_dilution <= 5)
    {
        // Good
        count_stars = 4;
        tft.setTextColor(0x3F6C, 0x0000);
    }
    else if(gps_posn.horizontal_dilution <= 10)
    {
        // Moderate
        count_stars = 3;
        tft.setTextColor(0xE705, 0x0000);
    }
    else if(gps_posn.horizontal_dilution <= 20)
    {
        // Fair
        count_stars = 2;
        tft.setTextColor(0xE525, 0x0000);
    }
    else
    {
        // Poor
        tft.setTextColor(0xE986, 0x0000);
        count_stars = 1;
    }
    
    // Draw starts up to the required number.
    int i = 0;
    while(i < count_stars)
    {
        tft.print("*");
        ++i;
    }
    
    // Draw spaces to clear/pad "empty" stars.
    while(i < 6)
    {
        tft.print(" ");
        ++i;
    }

    // Draw ending bar.
    tft.setTextColor(0xFFFF, 0x0000);
    tft.println("|");
}

/*
    Display a message to the user that unit is waiting for a GPS position.
*/
void display_wait()
{
    tft.setCursor(0, 0);
    tft.setTextColor(0xFFFF, 0x0000);
    tft.setTextSize(2);
    tft.println("Waiting for fix...");

    tft.print(F("Satellites: "));
    tft.print(gps_posn.number_satellites);
    tft.println("    ");
}

/*
    Indicate to the user that something went wrong with getting a GPS fix.
*/
void display_fail()
{
    tft.setCursor(0, 0);
    tft.setTextColor(0xFFFF, 0x0000);
    tft.setTextSize(2);
    tft.print("GPS Fail!");
}

/*
    Make an circle flash in the lower right corner of the screen every n seconds to indicate the unit is working.
*/
void flash_indicator()
{
    static unsigned long last_flash = 0;
    static bool flash_state = false;

    if(millis() - last_flash > FLASH_MS)
    {
        unsigned color;
        if(flash_state)
        {
            color = 0x33FE;
        }
        else
        {
            color = 0x0000;
        }
        
        tft.fillCircle(tft.width() - 15, tft.height() - 15, 7, color);
        last_flash = millis();
        flash_state = !flash_state;
    }
}