#include "gps_format.hpp"

using namespace Scottz0r;

bool GpsFormat::format_lat_ddmm(float deg, char* dst, size_t dst_size)
{
    // Required minimum buffer size.
    if (dst_size < 16)
    {
        return false;
    }

    // Valid latitude range check.
    if (deg < -90.0f || deg > 90.0f)
    {
        return false;
    }

    // North vs South. Always make the degrees absolute value to avoid negative signs.
    float abs_deg;
    if (deg < 0)
    {
        dst[0] = 'S';
        abs_deg = -1 * deg;
    }
    else
    {
        dst[0] = 'N';
        abs_deg = deg;
    }

    format_deg_ddmm(abs_deg, dst + 1, dst_size - 1);

    return true;
}

bool GpsFormat::format_lon_ddmm(float deg, char* dst, size_t dst_size)
{
    // Required minimum buffer size.
    if (dst_size < 16)
    {
        return false;
    }

    // Valid longitude range check
    if (deg < -180.0 || deg > 180.0)
    {
        return false;
    }

    // East vs West. Always make the degrees absolute value to avoid negative signs.
    float abs_deg;
    if (deg < 0)
    {
        dst[0] = 'E';
        abs_deg = -1 * deg;
    }
    else
    {
        dst[0] = 'W';
        abs_deg = deg;
    }

    // Pad leading zero if less than 3 digits.
    size_t idx = 1;
    if (abs_deg < 100.0)
    {
        dst[1] = '0';
        ++idx;
    }

    format_deg_ddmm(abs_deg, dst + idx, dst_size - idx);

    return true;
}

/**
    Format a number into degrees and minutes, like -DDD MM.MMMM.  Destination buffer must be 12 or more characters.
    Degrees must be between -180 and 180.
*/
bool GpsFormat::format_deg_ddmm(float deg, char* dst, size_t dst_size)
{
    if (dst_size < 13)
    {
        return false;
    }

    if (deg < -180.0 || deg > 180.0)
    {
        return false;
    }

    size_t idx = 0;
    float abs_deg;

    // Handle negative numbers by adding negative sign.
    if (deg < 0.0f)
    {
        abs_deg = -1 * deg;
        dst[0] = '-';
        ++idx;
    }
    else
    {
        abs_deg = deg;
    }

    // Format the whole degrees part.
    int number = (int)abs_deg;
    idx += int_to_string(number, dst + idx, dst_size - idx);

    dst[idx] = ' ';
    ++idx;

    // Format the whole number minutes.
    float minutes = (abs_deg - (int)abs_deg) * 60.0f;
    int minutes_whole = (int)minutes;

    idx += int_to_string(minutes_whole, dst + idx, dst_size - idx);

    dst[idx] = '.';
    ++idx;

    // Format a 4 digit decimal part of minutes.
    int minutes_dec = (int)((minutes - minutes_whole) * 10000.0f);
    idx = int_to_string(minutes_dec, dst + idx, dst_size - idx);

    return true;
}

void GpsFormat::reverse(char* buffer, int length)
{
    int start = 0;
    int end = length - 1;

    while (start < end)
    {
        buffer[start] ^= buffer[end];
        buffer[end] ^= buffer[start];
        buffer[start] ^= buffer[end];

        ++start;
        --end;
    }
}

size_t GpsFormat::int_to_string(int number, char* dst, size_t dst_size)
{
    if (dst_size == 0)
    {
        return 0;
    }

    size_t i = 0;
    size_t end = dst_size - 1;

    int temp_number = number;

    // Add negative sign to negative values.
    if (temp_number < 0)
    {
        dst[0] = '-';
        ++i;

        temp_number *= -1;
    }

    // Walk though the number, dividing by 10 to get each digit to add.
    while (temp_number != 0 && i < end)
    {
        int remainder = temp_number % 10;
        dst[i] = remainder + '0';
        ++i;
        temp_number /= 10;
    }

    // For negative numbers, do not include the negative sign when reversing.
    if (number < 0)
    {
        reverse(dst + 1, i - 1);
    }
    else
    {
        reverse(dst, i);
    }

    dst[i] = 0;

    return i;
}
