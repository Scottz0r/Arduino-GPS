#include "prj_display.h"
#include <MicroGpsFormat.h>

using namespace scottz0r::gps;

constexpr auto FLASH_MS = 1000; // Flash delay for active indicator.

#define MAX_PRINT_LINES 10

TftDisplay::TftDisplay(int cs, int dc, int rst)
    : m_tft(cs, dc, rst)
    , m_state(DisplayState::PrintMode)
    , m_wait_start(0)
    , m_render_state(PositionRenderState::Start)
    , m_gps_position({})
    , m_last_flash(0)
    , m_flash_state(false)
    , m_print_lines(0)
{   
}

void TftDisplay::init()
{
    m_tft.init(135, 240);

    // Have to do 0 rotation to reset all pixels due to a bug in the TFT library that doesn't set position properly
    // when the screen is rotated.
    m_tft.setRotation(0);
    m_tft.fillScreen(0x0000);

    m_tft.setRotation(1);
}

void TftDisplay::process()
{
    // Only do processing ticks / incremental screen updates for specific types.
    if(m_state == DisplayState::Position)
    {
        process_gps_position();
    }

    // Always process the flashing indicator.
    process_indicator();
}

void TftDisplay::display_position(const GpsPosition& gps_position)
{
    // Copy data to this object because GPS position data could change while display is getting updated.
    m_gps_position = gps_position;

    // Clear entire screen if the state completely changed.
    if(m_state != DisplayState::Position)
    {
        clear_screen();
    }

    // Switch state to show position data.
    m_state = DisplayState::Position;

    // Reset processing state for dispaly back to beginning.
    m_render_state = PositionRenderState::Start;
}

void TftDisplay::display_wait()
{
    if(m_state != DisplayState::Waiting)
    {
        clear_screen();

        m_tft.setCursor(0, 0);
        m_tft.setTextColor(0xFFFF, 0x0000);
        m_tft.setTextSize(2);
        m_tft.println("Waiting for fix...");
    }

    m_state = DisplayState::Waiting;
}

void TftDisplay::display_fail()
{
    const char FAIL_MSG[] = "GPS FAIL";

    // Do not redraw screen if state is already in Fail mode.
    if(m_state == DisplayState::Fail)
    {
        return;
    }

    clear_screen();

    // Draw a yellow triangle on the top half of the screen.
    uint16_t x0, y0, x1, y1, x2, y2, h;

    // Height is half of the screen.
    h = m_tft.height() / 2;

    uint16_t half_width = m_tft.width() / 2;
    uint16_t half_height = (m_tft.height() / 2);

    // x1, y1 will be at the top center of the screen.
    x1 = half_width;
    y1 = 1;

    // For an issolies triangle, h = sqrt(3)/2 * a, so a = 2/sqrt(3) * h
    // Also, is 2 * h / tan(60) = 2 * h / sqrt(3).
    // 2 / sqrt(3) == approx 1.1547
    // To put the triangle in the middle, take (width() / 2) +/- ((h * 1.1547) / 2)
    uint16_t half_side_length = (uint16_t)(h * 1.547f / 2.0f);

    x0 = half_width - half_side_length;
    y0 = h;

    x2 = half_width + half_side_length;
    y2 = h;

    m_tft.fillTriangle(x0, y0, x1, y1, x2, y2, 0xFFE0);

    // Draw an exclamation mark in the middle.
    x0 = half_width - 7;
    y0 = (half_height / 2) - 4;
    m_tft.drawChar(x0, y0, '!', 0x0000, 0xFFE0, 3);

    // Render the message under the triangle (2px padding)
    // Must set text size before computations!
    m_tft.setTextSize(2);

    uint16_t cursor_y = half_height + 4;

    int16_t not_used_s;
    uint16_t txt_h, txt_w;
    m_tft.getTextBounds(FAIL_MSG, 0, 0, &not_used_s, &not_used_s, &txt_w, &txt_h);

    // Center text by taking half screen width minus half the text width.
    uint16_t cursor_x = half_width - (txt_w / 2);

    // Finally, render the text in the center of the screen.
    m_tft.setCursor(cursor_x, cursor_y);
    m_tft.setTextColor(0xFFFF);
    m_tft.print(FAIL_MSG);

    m_state = DisplayState::Fail;
}

void TftDisplay::start_print_mode()
{
    m_state = DisplayState::PrintMode;
    clear_screen();
    m_tft.setCursor(0, 0);
    m_tft.setTextSize(1);
    m_tft.setTextColor(0xFFFF, 0x0000);
}

void TftDisplay::println(const __FlashStringHelper* msg)
{
    if(m_state != DisplayState::PrintMode)
    {
        return;
    }

    if(m_print_lines > MAX_PRINT_LINES)
    {
        // TODO: A better method would be knowing how tall screen is and inspecing where cursor is.
        clear_screen();
        m_tft.setCursor(0, 0);
    }

    m_tft.println(msg);
    ++m_print_lines;
}

void TftDisplay::print(const __FlashStringHelper* msg)
{
    if(m_state != DisplayState::PrintMode)
    {
        return;
    }

    m_tft.print(msg);
}

void TftDisplay::clear_screen()
{
    m_tft.fillScreen(0x0000);
}

/// @brief Display GPS position data, processing data in chunks because rendering is slow.
void TftDisplay::process_gps_position()
{
    if(m_render_state == PositionRenderState::Start)
    {
        m_tft.setCursor(0, 0);
        m_tft.setTextColor(0xFFFF, 0x0000);
        m_tft.setTextSize(2);

        char fmt_buffer[32];
        format_lat_ddmm(m_gps_position.latitude, fmt_buffer, sizeof(fmt_buffer));
        m_tft.println(fmt_buffer);

        format_lon_ddmm(m_gps_position.longitude, fmt_buffer, sizeof(fmt_buffer));
        m_tft.println(fmt_buffer);

        m_render_state = PositionRenderState::Other;
    }
    else if(m_render_state == PositionRenderState::Other)
    {
        // Print other GPS stuff, and pad at end to account for changes magnitude of numbers.
        m_tft.print(F("Satellites: "));
        m_tft.print(m_gps_position.number_satellites);
        m_tft.println(F("    "));

        m_tft.print(F("Altitude: "));
        m_tft.print(m_gps_position.altitude_msl);
        m_tft.println(F("    "));

        m_tft.setTextSize(1);
        m_tft.print(F("Geoidal separation: "));
        m_tft.print(m_gps_position.geoid_height);
        m_tft.println(F("    "));

        m_render_state = PositionRenderState::HDOP;
    }
    else if(m_render_state == PositionRenderState::HDOP)
    {
        // Make a pretty "Bar" for HDOP. Should make a real graphic in the future.
        display_hdop_bar();
        m_render_state = PositionRenderState::Done;
    }
}

void TftDisplay::display_hdop_bar()
{
    m_tft.setTextSize(2);
    m_tft.setTextColor(0xFFFF, 0x0000);
    m_tft.print("HDOP: |");

    // Based off of the dilution, display a number of stars where 6 = best and 1 = worst.
    // Change colors based off the dilution, too.
    int count_stars = 0;
    if(m_gps_position.horizontal_dilution <= 1)
    {
        // Ideal
        count_stars = 6;
        m_tft.setTextColor(0x3F6C, 0x0000);
    }
    else if(m_gps_position.horizontal_dilution <= 2)
    {
        // Excellent
        count_stars = 5;
        m_tft.setTextColor(0x3F6C, 0x0000);
    }
    else if(m_gps_position.horizontal_dilution <= 5)
    {
        // Good
        count_stars = 4;
        m_tft.setTextColor(0x3F6C, 0x0000);
    }
    else if(m_gps_position.horizontal_dilution <= 10)
    {
        // Moderate
        count_stars = 3;
        m_tft.setTextColor(0xE705, 0x0000);
    }
    else if(m_gps_position.horizontal_dilution <= 20)
    {
        // Fair
        count_stars = 2;
        m_tft.setTextColor(0xE525, 0x0000);
    }
    else
    {
        // Poor
        m_tft.setTextColor(0xE986, 0x0000);
        count_stars = 1;
    }
    
    // Draw starts up to the required number.
    int i = 0;
    while(i < count_stars)
    {
        m_tft.print("*");
        ++i;
    }
    
    // Draw spaces to clear/pad "empty" stars.
    while(i < 6)
    {
        m_tft.print(" ");
        ++i;
    }

    // Draw ending bar.
    m_tft.setTextColor(0xFFFF, 0x0000);
    m_tft.println("|");
}

/// @brief Make an circle flash in the lower right corner of the screen every n seconds to indicate the unit is working.
void TftDisplay::process_indicator()
{
    if(millis() - m_last_flash > FLASH_MS)
    {
        unsigned color;
        if(m_flash_state)
        {
            color = 0x33FE;
        }
        else
        {
            color = 0x0000;
        }
        
        m_tft.fillCircle(m_tft.width() - 15, m_tft.height() - 15, 7, color);
        m_last_flash = millis();
        m_flash_state = !m_flash_state;
    }
}
