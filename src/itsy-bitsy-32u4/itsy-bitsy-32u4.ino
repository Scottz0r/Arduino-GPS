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
#include "message_watchdog.h"

using namespace scottz0r::gps;

auto gps = MicroGps();

// Extern declared in prj_display.
TftDisplay tft_display = TftDisplay(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

MessageWatchdog gps_position_watchdog(SERIAL_WATCHDOG_TIMEOUT_MS);

static void process_gps_serial();
static void startup_fail_state();
static void hard_fail();

void setup()
{
    // Configure pins
    pinMode(PIN_GPS_ENABLE, OUTPUT);

    tft_display.init();

    // Enter print mode when starting up the GPS.
    tft_display.start_print_mode();

    mtk3339::ReturnCode init_rc = mtk3339::init();
    switch(init_rc)
    {
    case mtk3339::ReturnCode::StartupError:
        tft_display.println(F("GPS startup failed!"));
        startup_fail_state();
        break;
    case mtk3339::ReturnCode::ConfigError:
        tft_display.println(F("GPS configuration failed!"));
        startup_fail_state();
        break;
    }

    // Start in a waiting for position state.
    // TODO: Maybe make this a splash screen?
    tft_display.display_wait();

    // Kick watchdog to start it off.
    gps_position_watchdog.kick();
}

void loop()
{
    process_gps_serial();

    tft_display.process();

    // If the GPS serial watchdog has expired, then go to an error state.
    if(gps_position_watchdog.is_expired())
    {
        hard_fail();
    }
}

/// @brief Process data incoming on the GPS Serial connection.
static void process_gps_serial()
{
    static int sequential_bad_messages = 0;

    // Collect NMEA messages from GPS serial device.
    while(GpsSerial.available())
    {
        char c = GpsSerial.read();
        if(gps.process(c))
        {
            // Any message has been received. Kick the watchdog.
            gps_position_watchdog.kick();

            if(gps.good())
            {
                // Reset counter of sequential bad messages once a good message is received.
                sequential_bad_messages = 0;

                if(gps.message_type() == MicroGps::MessageType::GPGGA)
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
            }
            else
            {
                // If the maximum number of sequential bad messages encountered, go to a hard fail state.
                ++sequential_bad_messages;
                if(sequential_bad_messages > MAX_BAD_NMEA_MESSAGES)
                {
                    hard_fail();
                }
            }

            // Stop looping if a full message has been parsed. To allow other items to process.
            break;
        }
    }    
}

/// @brief Put the unit into an infinite loop after a setup fail.
static void startup_fail_state()
{
    tft_display.println(F("GPS has failed to start."));
    tft_display.println(F("Restart Unit!"));

    for(;;)
    { }
}

/// @brief Put the unit into a hard fail state that displays an error screen and goes into an infinite loop.
static void hard_fail()
{
    tft_display.display_fail();

    for(;;)
    { }
}
