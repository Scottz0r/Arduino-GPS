#include "nmea_parser.hpp"

using namespace ScottZ0r;

int main()
{
    const char msg_0[] = "$GPGGA,153845.307,,,,,0,00,,,M,,M,,*72\r\n";
    const char msg_1[] = "$GPGGA,153903.137,3854.8669,N,09445.3785,W,1,03,2.37,180.1,M,-30.1,M,,*5F";
    const char msg_2[] = "$GPGGA,153903.000,3854.8669";


    bool checksum_0 = NmeaParser::checksum(msg_0, sizeof(msg_0) - 1);
    bool checksum_1 = NmeaParser::checksum(msg_1, sizeof(msg_1) - 1);
    bool checksum_3 = NmeaParser::checksum(msg_1, sizeof(msg_2));

    bool success = false;
    GpsPosition posn;

    success = NmeaParser::parse_gga(msg_0, sizeof(msg_0), posn);

    success = NmeaParser::parse_gga(msg_1, sizeof(msg_1) - 1, posn);

    success = NmeaParser::parse_gga(msg_2, sizeof(msg_2), posn);

    return 0;
}