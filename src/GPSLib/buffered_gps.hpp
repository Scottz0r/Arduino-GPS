#ifndef _SZR_BUFFERED_GPS_INCLUDE_GUARD
#define _SZR_BUFFERED_GPS_INCLUDE_GUARD

#include <arduino.h>

#include "prj_gps_serial.hpp"

namespace Scottz0r
{
    class BufferedGps
    {
        static constexpr auto BUFFER_SIZE = 256;

        enum class State
        {
            WaitStart,
            CollectSentence,
            SentenceReady
        };
    public:
        BufferedGps();

        void begin();

        virtual bool configure() { return true; };

        const char* sentence() const;

        void process();

        bool sentence_available() const;

        size_t sentence_size() const;

        bool fail() const;

        void reset_fail();

        void clear_sentence();

        bool wait_for_sentence(unsigned long timeout);

    protected:
        GpsSerial m_serial;

    private:
        void start_collecting_sentence();

        bool m_fail;
        State m_state;

        char m_buffer[BUFFER_SIZE];
        char* m_p_buffer;
    };
}

#endif // _SZR_BUFFERED_GPS_INCLUDE_GUARD
