#ifndef _SCOTTZ0R_PRJ_GPS_SERIAL_INCLUDE_GUARD
#define _SCOTTZ0R_PRJ_GPS_SERIAL_INCLUDE_GUARD

namespace Scottz0r
{
    /**
        Wrapper class to provide a layer of abstraction from a project specific interfaces.
        This defines the methods needed from a projet Serial interface, wether it be software
        or hardware.

        Implementations of this class should be as thing as possible.
    */
    class GpsSerial
    {
    public:
        int available();

        int read();
    };

}

#endif // _SCOTTZ0R_PRJ_GPS_SERIAL_INCLUDE_GUARD
