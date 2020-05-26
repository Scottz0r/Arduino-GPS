#ifndef _ITSY_BITSY_32U4_PRJ_DISPLAY_INCLUDE_GUARD_H
#define _ITSY_BITSY_32U4_PRJ_DISPLAY_INCLUDE_GUARD_H

#include <Adafruit_ST7789.h>
#include <arduino.h>
#include <MicroGps.h>

class TftDisplay
{
    enum class DisplayState
    {
        Startup,
        Waiting,
        Position,
        Fail
    };

    enum class PositionRenderState
    {
        Start,
        Other,
        HDOP,
        Done
    };

public:
    TftDisplay(int cs, int dc, int rst);

    void init();

    void process();

    void display_position(const scottz0r::gps::GpsPosition& gps_position);

    void display_wait();

    void display_fail();

    // TODO: Splash screen / startup?

private:
    void clear_screen();

    void process_gps_position();

    void display_hdop_bar();

    void process_indicator();

    Adafruit_ST7789 m_tft;

    DisplayState m_state;

    // For waiting state:
    unsigned long m_wait_start;

    // For GPS Position state:
    PositionRenderState m_render_state;
    scottz0r::gps::GpsPosition m_gps_position;

    // For process_indicator
    unsigned long m_last_flash = 0;
    bool m_flash_state = false;
};

#endif // _ITSY_BITSY_32U4_PRJ_DISPLAY_INCLUDE_GUARD_H
