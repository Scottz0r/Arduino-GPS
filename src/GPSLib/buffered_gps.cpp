#include "buffered_gps.hpp"
#include <arduino.h>

using namespace Scottz0r;

/**
    Initialize the buffered Gps reader by assigning pins. Initializes underlying software serial.
*/
BufferedGps::BufferedGps()
    : m_fail(false)
    , m_state(State::WaitStart)
{
    m_p_buffer = m_buffer;
}

/**
    Setup the buffered reader. This must be called before using the object.
*/
void BufferedGps::begin()
{

}

/**
    Get the available NMEA sentence from the internal buffer. Returns nullptr if a sentence is not ready.
*/
const char* BufferedGps::sentence() const
{
    if (m_state == State::SentenceReady)
    {
        return m_buffer;
    }
    else
    {
        return nullptr;
    }
}

/**
    Returns true if a NMEA sentence is available.
*/
bool BufferedGps::sentence_available() const
{
    return m_state == State::SentenceReady;
}

/**
    Returns the number of bytes in the current sentence, not including null terminators.
*/
size_t BufferedGps::sentence_size() const
{
    if (m_state == State::SentenceReady)
    {
        return m_p_buffer - m_buffer;
    }
    else
    {
        return 0;
    }
}

/**
    Process "tick" for this object. This polls the serial device for available characters. This is non-blocking.
*/
void BufferedGps::process()
{
    // No-op if serial data is not available.
    if (!m_serial.available())
    {
        return;
    }

    char c = m_serial.read();

    if (m_state == State::WaitStart)
    {
        if (c == '$')
        {
            start_collecting_sentence();
            m_state = State::CollectSentence;
        }
        else
        {
            m_fail = true;
        }
    }
    else if (m_state == State::CollectSentence)
    {
        // Check for overflow condtiion. Account for a null terminator at end (hence -2). Reset if overflowed. Must go
        // back to waiting for a valid start character after the failure.
        if (m_p_buffer - m_buffer >= BUFFER_SIZE - 2)
        {
            m_fail = true;
            m_p_buffer = m_buffer;
            m_state = State::WaitStart;
        }
        else if (c == '$')
        {
            // Getting a $ here indicates a new NMEA sentence is available, which is unexpected.
            start_collecting_sentence();
            m_fail = true;
        }
        else
        {
            // Collect the character as normal.
            *m_p_buffer = c;
            ++m_p_buffer;

            // Newline indicates end of sentence. Mark as available after this.
            if (c == '\n')
            {
                *m_p_buffer = 0;
                m_state = State::SentenceReady;
                // m_is_available = true; // Don't need this with states
            }
        }

    }
    else if (m_state == State::SentenceReady)
    {
        // If a new sentence character is present, then switch to collectin the sentence. Otherwise, something happened,
        // and the serial messages are not vaild, so go back to waiting for a start sentence.
        if (c == '$')
        {
            start_collecting_sentence();
            m_state = State::CollectSentence;
        }
        else
        {
            m_state = State::WaitStart;
            m_fail = true;
        }
    }
}

/**
    Returns true if a process action failed. This will remain true until reset_fail() is called. This will be
    false after object initialization.
*/
bool BufferedGps::fail() const
{
    return m_fail;
}

/**
    Reset the fail flag state to false.
*/
void BufferedGps::reset_fail()
{
    m_fail = false;
}

/**
    Clear the sentence state of this objecct. Once cleared, the object will stop reporting a message is ready.
    
    Call this after processing a sentence to prevent processing the same sentence multiple times.
*/
void BufferedGps::clear_sentence()
{
    m_p_buffer = m_buffer;
    m_state = State::WaitStart;
}

/**
    Blocking wait for a complete sentence. Returns true if a sentence is available. Returns false if a timeout
    occurred.
*/
bool BufferedGps::wait_for_sentence(unsigned long timeout)
{
    unsigned long start = millis();

    while (true)
    {
        process();
        if (sentence_available())
        {
            return true;
        }

        if (millis() - start >= timeout)
        {
            return false;
        }
    }

    return false;
}

/**
    Set the internal buffer into a state that is collecting a NMEA sentence. The first character will always be a '$'.
*/
void BufferedGps::start_collecting_sentence()
{
    m_p_buffer = m_buffer;
    *m_p_buffer = '$';
    ++m_p_buffer;
}
