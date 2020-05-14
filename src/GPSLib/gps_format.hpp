#ifndef _SCOTTZ0R_GPS_FORMAT_INCLUDE_GUARD
#define _SCOTTZ0R_GPS_FORMAT_INCLUDE_GUARD

#include <arduino.h>

namespace Scottz0r
{
    class GpsFormat
    {
    public:
        static bool format_lat_ddmm(float deg, char* dst, size_t dst_size);

        static bool format_lon_ddmm(float deg, char* dst, size_t dst_size);

        static bool format_deg_ddmm(float deg, char* dst, size_t dst_size);

    private:
        static void reverse(char* buffer, int length);

        static size_t int_to_string(int number, char* dst, size_t dst_size);
    };
}

#endif // _SCOTTZ0R_GPS_FORMAT_INCLUDE_GUARD
