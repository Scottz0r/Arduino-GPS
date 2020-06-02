#include "message_watchdog.h"
#include <Arduino.h>

/// @brief Initialize a watchdog time with a specific timeout, in milliseconds.
///
/// @param timeout Watchdog timer timeout.
MessageWatchdog::MessageWatchdog(time_type timeout)
    : m_timeout(timeout)
    , m_last_kicked(0)
{
}

/// @brief Returns true if the timer has expired.
///
/// @return True if watchdog timer has expired.
bool MessageWatchdog::is_expired() const
{
    return millis() - m_last_kicked > m_timeout;
}

/// @brief Kick the watchdog, resetting the last kicked and logically restarting the timer.
void MessageWatchdog::kick()
{
    m_last_kicked = millis();
}
