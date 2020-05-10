#ifndef _SCOTTZ0R_NMEA_PARSER_INCLUDE_GUARD
#define _SCOTTZ0R_NMEA_PARSER_INCLUDE_GUARD

#include <arduino.h>
#include <math.h>

namespace ScottZ0r
{
    struct GpsPosition
    {
        bool has_fix;
        int timestamp;
        float latitude;
        float longitude;
        int number_satellites;
        float horizontal_dilution;
        float altitude_msl;
        float altitude_wgs_84;
    };

    class NmeaParser
    {
    public:
        static bool parse_gga(const char* message, size_t count, GpsPosition& result);

        static bool checksum(const char* message, size_t count);
    };
}

#endif // _SCOTTZ0R_NMEA_PARSER_INCLUDE_GUARD