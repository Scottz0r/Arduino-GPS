#ifndef _SCOTTZ0R_NMEA_PARSER_INCLUDE_GUARD
#define _SCOTTZ0R_NMEA_PARSER_INCLUDE_GUARD

#include <arduino.h>

namespace ScottZ0r
{
    struct GpsPosition
    {
        int timestamp;
        long latitude;
        long longitude;
    };

    class NmeaParser
    {
    public:
        static bool parse_gga(const char* message, size_t count, GpsPosition& result);

        static bool checksum(const char* message, size_t count);
    };
}

#endif // _SCOTTZ0R_NMEA_PARSER_INCLUDE_GUARD