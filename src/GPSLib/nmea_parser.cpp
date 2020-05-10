#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "nmea_parser.hpp"

using namespace ScottZ0r;

static int hex_to_int(char val);

static const char* split_comma(const char* val, size_t& count, char* dst, size_t dst_size);
static bool parse_timestamp(const char* val, int& result);

static inline bool is_digit(int c);
static inline int char_to_int(int c);

static const bool parse_latitude(const char* val, const char* dir, float& result, bool& has_posn);
static const bool parse_longitude(const char* val, const char* dir, float& result, bool& has_posn);

bool NmeaParser::parse_gga(const char* message, size_t count, GpsPosition& result)
{
    // TODO - really parse entire message.
    const char* p_field = message;
    size_t remain_count = count;
    char buffer[32];
    char dir_buffer[4];

    bool valid = true;
    result = {};

    // Message Id. Confirm this is $GPGGA. If not, doesn't even try to parse.
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));
    if (strcmp(buffer, "$GPGGA") != 0)
    {
        return false;
    }

    // Time
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));
    parse_timestamp(buffer, result.timestamp);

    // TODO: Maybe make two buffers (I don't need 32 char buffers anyway), and parse lat and lat dir at same time.
    // Latitude
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));
    p_field = split_comma(p_field, remain_count, dir_buffer, sizeof(dir_buffer));

    bool has_latitude = false;
    if (!parse_latitude(buffer, dir_buffer, result.latitude, has_latitude))
    {
        valid = false;
    }

    // Longitude
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));
    p_field = split_comma(p_field, remain_count, dir_buffer, sizeof(dir_buffer));

    bool has_longitude = false;
    if (!parse_longitude(buffer, dir_buffer, result.longitude, has_longitude))
    {
        valid = false;
    }

    // If either lat or longitude didn't have a value, then consider entire message to not have a fix.
    if (!has_longitude || !has_latitude)
    {
        result.latitude = 0.0f;
        result.longitude = 0.0f;
        result.has_fix = false;
    }
    else
    {
        result.has_fix = true;
    }

    // Fix quality
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));

    // Number of satellites
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));
    result.number_satellites = atol(buffer);

    // Horizontal dilution of position
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));
    result.horizontal_dilution = (float)atof(buffer);

    // Altitude
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));
    result.altitude_msl = (float)atof(buffer);

    // Altitude units. TODO: Always M?
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));

    // Height above WGS-84 ellipsoid
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));
    result.altitude_wgs_84 = (float)atof(buffer);

    // Height units. TODO: Always M?
    p_field = split_comma(p_field, remain_count, buffer, sizeof(buffer));


    // Reset output state if not valid.
    if (!valid)
    {
        result = {};
        return false;
    }

    return true;
}

/**
    Compute and compare the checksum of the given message. NMEA messages must be in the format of
    "$<content>*HH", where HH is the hex checksum value. Items between but not including $ and * will
    be XOR'ed to compute the checksum.

    @param message The NMEA message to verify the checksum on.
    @param count The number of characters in the message.
    @return True if checksum passed.
*/
bool NmeaParser::checksum(const char* message, size_t count)
{
    if (!message)
    {
        return false;
    }

    if (count == 0)
    {
        return false;
    }

    // Valid NMEA must start with a $.
    if (*message != '$')
    {
        return false;
    }

    const char* p_message = message + 1;
    size_t idx = 1;
    bool found_checksum_flag = false;

    char checksum = 0;

    while (*p_message != 0 && idx < count && !found_checksum_flag)
    {
        if (*p_message == '*')
        {
            found_checksum_flag = true;;
        }
        else
        {
            checksum ^= *p_message;
        }

        ++p_message;
        ++idx;
    }

    // No checksum flag means checksum failed.
    if (!found_checksum_flag)
    {
        return false;
    }

    // Next two characters after checksum flag is 1 byte hex (two nibble chars).
    char message_checksum = 0;
    if (idx + 1 < count)
    {
        int checksum_msb = hex_to_int(*p_message);
        ++p_message;
        int checksum_lsb = hex_to_int(*p_message);
        message_checksum = checksum_msb << 4 | checksum_lsb;
    }

    return checksum == message_checksum;
}

static int hex_to_int(char val)
{
    if (val >= '0' && val <= '9')
    {
        return val - '0';
    }
    else if (val >= 'a' && val <= 'f')
    {
        return val - 'a' + 10;
    }
    else if (val >= 'A' && val <= 'F')
    {
        return val - 'A' + 10;
    }
    else
    {
        return 0;
    }
}

/**
    Copy characters into dst up to the first comma. Returns a pointer to the character after the comma, if found, or
    nullptr if a comma was not found.

    Updates count variable with the number of characters remaining past the comma, if found, or zero if the end of the
    string was reached.

    Must supply buffer sizes for safe parsing. Destination buffer will always be null terminated.
*/
static const char* split_comma(const char* val, size_t& count, char* dst, size_t dst_size)
{
    // Guard against bad inputs. Return an end state if any input is invalid.
    if (val == nullptr || count == 0 || dst == nullptr || dst_size == 0)
    {
        count = 0;
        return nullptr;
    }

    size_t idx = 0;
    bool found = false;

    while ((val[idx] != 0) && (idx < count) && (idx < (dst_size - 1)) && !found)
    {
        if (val[idx] == ',')
        {
            found = true;
        }
        else
        {
            dst[idx] = val[idx];
            ++idx;
        }
    }

    dst[idx] = 0;

    // Advance past comma to return start of the next field.
    if (found)
    {
        ++idx;
        count -= idx;
        return val + idx;
    }
    else
    {
        // A comma was not found, or the end of the string was hit, so return nullptr to indicate this.
        count = 0;
        return nullptr;
    }
}

static const bool parse_latitude(const char* val, const char* dir, float& result, bool& has_posn)
{
    result = 0.0f;
    has_posn = false;

    if (val == nullptr || dir == nullptr)
    {
        // Invalid.
        return false;
    }

    if (dir[0] != 'N' && dir[0] != 'S' && dir[0] != 0)
    {
        // Invalid.
        return false;
    }

    // Return if degrees is not at least two characters or no direction.
    if (val[0] == 0 || val[1] == 0 || dir[0] == 0)
    {
        // No fix, but valid parse.
        return true;
    }

    has_posn = true;

    // First two characters are degrees.
    char deg_buff[3];
    deg_buff[0] = val[0];
    deg_buff[1] = val[1];
    deg_buff[2] = 0;

    // Parse degrees.
    float deg = (float)atof(deg_buff);

    // Collect the minutes part and convert to degrees.
    float minutes = (float)atof(val + 2);
    minutes /= 60.0f;

    result = deg + minutes;

    if (dir[0] == 'S')
    {
        result *= -1.0f;
    }

    return true;
}

static const bool parse_longitude(const char* val, const char* dir, float& result, bool& has_posn)
{
    result = 0.0f;
    has_posn = false;

    if (val == nullptr || dir == nullptr)
    {
        return false;
    }

    if (dir[0] != 'E' && dir[0] != 'W' && dir[0] != 0)
    {
        // Invalid.
        return false;
    }

    // Return if degrees is not at least three characters.
    if (val[0] == 0 || val[1] == 0 || val[2] == 0 || dir[0] == 0)
    {
        // No fix, but valid parse.
        return true;
    }

    has_posn = true;

    // First three characters are degrees.
    char deg_buff[4];
    deg_buff[0] = val[0];
    deg_buff[1] = val[1];
    deg_buff[2] = val[2];
    deg_buff[3] = 0;

    // Parse degrees.
    float deg = (float)atof(deg_buff);

    // Collect the minutes part and convert to degrees.
    float minutes = (float)atof(val + 3);
    minutes /= 60.0f;

    result = deg + minutes;

    if (dir[0] == 'W')
    {
        result *= -1.0f;
    }

    return true;
}

static bool parse_timestamp(const char* val, int& result)
{
    int idx = 0;
    while (val[idx] != 0 && idx < 6)
    {
        if (is_digit(val[idx]))
        {
            result *= 10;
            result += char_to_int(val[idx]);
        }
        else
        {
            break;
        }

        ++idx;
    }

    if (idx < 6)
    {
        result = 0;
        return false;
    }

    return true;
}

static inline bool is_digit(int c)
{
    return c >= '0' && c <= '9';
}

static inline int char_to_int(int c)
{
    if (is_digit(c))
    {
        return c - '0';
    }

    return 0;
}