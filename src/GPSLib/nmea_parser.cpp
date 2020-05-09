#include "nmea_parser.hpp"

using namespace ScottZ0r;

static int hex_to_int(char val);

static char* split_comma(const char* val, size_t count, char* dst, size_t buffer_size);

bool NmeaParser::parse_gga(const char* message, size_t count, GpsPosition& result)
{
    return false;
}

/**
    Compute and compare the checksum of the given message. NMEA messages must be in the format of
    "$<content>*HH", where HH is the hex checksum value. Items between but not including $ and * will
    be XOR'ed toc compute the checksum.

    @param message The NMEA message to verify the checksum on.
    @param count The number of charcters in the message. May include or disclude null terminator.
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

static char* split_comma(const char* val, size_t count, char* dst, size_t buffer_size)
{

}