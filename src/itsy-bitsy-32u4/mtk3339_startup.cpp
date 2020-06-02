#include "mtk3339_startup.h"
#include "prj_display.h"
#include "prj_pins.h"

#define GPS_SETUP_TIMEOUT_MS 2000

namespace mtk3339
{
static void collect_message(char *buffer, size_type buffer_size, time_type timeout);
static bool wait_for_startup_messages(time_type timeout);
static bool wait_for_exact_msg(const char *msg, time_type timeout);

/// Set up the MTK3339 GPS module. This will wait for system startup messages and send configuration messages.
/// Error codes will be returned if setup was not successful. This method blocks until setup and ACK messages are
/// received, or until a timeout.
ReturnCode init()
{
    static const char mtk314_ack[] = "$PMTK001,314,3*36";
    static const char mtk220_ack[] = "$PMTK001,220,3*30";

    bool setup_rc;

    // Write 0 to the GPS Enable pin, ensuring a shutdown of the GPS module.
    tft_display.print(F("GPS OFF..."));
    digitalWrite(PIN_GPS_ENABLE, LOW);

    // Start serial ith default baud of 9600.
    GpsSerial.begin(9600);

    // Write 1 to the GPS Enable pin, powring up the GPS
    delay(500);
    tft_display.println(F("Now ON!"));
    digitalWrite(PIN_GPS_ENABLE, HIGH);

    // Wait for the special startup messages.
    tft_display.println(F("Waiting for GPS startup..."));
    setup_rc = wait_for_startup_messages(GPS_SETUP_TIMEOUT_MS);

    if (!setup_rc)
    {
        tft_display.println(F("Did not find gps startup messages!"));
        return ReturnCode::StartupError;
    }
    tft_display.println(F("GPS Startup OK"));

    // Configure to send GGA only.
    tft_display.println(F("Configuring 314..."));
    GpsSerial.println(F("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"));
    setup_rc = wait_for_exact_msg(mtk314_ack, GPS_SETUP_TIMEOUT_MS);

    if (!setup_rc)
    {
        tft_display.println(F("314 message did not ACK!"));
        return ReturnCode::ConfigError;
    }
    tft_display.println(F("Configure 314 OK"));

    // Configure to send at a 5 second interval.
    tft_display.println(F("Configuring 220... "));
    GpsSerial.println(F("$PMTK220,5000*1B"));
    setup_rc = wait_for_exact_msg(mtk220_ack, GPS_SETUP_TIMEOUT_MS);

    if (!setup_rc)
    {
        tft_display.println(F("220 message did not ACK!"));
        return ReturnCode::ConfigError;
    }
    tft_display.println(F("Configure 220 OK"));

    tft_display.println(F("Setup completed Ok!"));

    tft_display.println(F("System will start in 5 seconds..."));
    tft_display.print(F("5 "));
    delay(1000);
    tft_display.print(F("4 "));
    delay(1000);
    tft_display.print(F("3 "));
    delay(1000);
    tft_display.print(F("2 "));
    delay(1000);
    tft_display.print(F("1 "));
    delay(1000);
    tft_display.println(F("BLAST OFF!!!"));

    return ReturnCode::Ok;
}

/// Collect a message in the buffer up to the given size. Result will always be null terminated. This will not
/// collect newline or carriage return characters.
///
/// This is a blocking method.
static void collect_message(char *buffer, size_type buffer_size, time_type timeout)
{
    size_type idx = 0;
    time_type start = millis();
    time_type elapsed = 0;

    while (elapsed < timeout)
    {
        if (GpsSerial.available())
        {
            char c = GpsSerial.read();

            // Don't collect newline or carriage returns to make string comparisons easier.
            if (c != '\n' && c != '\r' && idx < buffer_size)
            {
                buffer[idx] = c;
                ++idx;
            }

            // A NMEA message stops at a newline, so break out of the process loop here.
            if (c == '\n')
            {
                break;
            }
        }

        elapsed = millis() - start;
    }

    // Always null terminate output.
    if (idx < buffer_size)
    {
        buffer[idx] = 0;
    }
    else
    {
        buffer[buffer_size - 1] = 0;
    }
}

/// Wait for the two special messages that are sent on GPS startup. There are also some other messages that get sent
/// and are not in the spec, so wait for the specific message strings.
///
/// This is a blocking method.
static bool wait_for_startup_messages(time_type timeout)
{
    static const char mtk_010[] = "$PMTK010,001*2E";
    static const char mtk_011[] = "$PMTK011,MTKGPS*08";

    char buffer[32];
    int found_flags = 0;

    time_type start = millis();
    time_type elapsed = 0;

    while (elapsed < timeout)
    {
        // Blocking collect of messages.
        collect_message(buffer, sizeof(buffer), timeout);

        if (strcmp(buffer, mtk_010) == 0)
        {
            found_flags |= 1;
        }
        else if (strcmp(buffer, mtk_011) == 0)
        {
            found_flags |= 2;
        }
        else
        {
            tft_display.println(F("(Startup found other message)"));
            // tft_display.println(buffer);
        }

        if (found_flags == 3)
        {
            return true;
        }

        elapsed = millis() - start;
    }

    return false;
}

/// Wait for an exact message from the GPS serial. If a message is not found within the timeout period, false is
/// returned.
///
/// This is a blocking method.
static bool wait_for_exact_msg(const char *msg, time_type timeout)
{
    char buffer[32];
    time_type start = millis();
    time_type elapsed = 0;

    while (elapsed < timeout)
    {
        collect_message(buffer, sizeof(buffer), timeout);

        if (strcmp(buffer, msg) == 0)
        {
            return true;
        }
        else
        {
            // Other message found.
        }

        elapsed = millis() - start;
    }

    return false;
}
} // namespace mtk3339
