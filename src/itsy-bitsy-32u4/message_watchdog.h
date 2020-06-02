#ifndef _MESSAGE_WATCHDOG_INCLUDE_GUARD
#define _MESSAGE_WATCHDOG_INCLUDE_GUARD

/// Defines a class for a simple watchdog.
class MessageWatchdog
{
    using time_type = unsigned long;

public:
    MessageWatchdog(time_type timeout);

    bool is_expired() const;

    void kick();

private:
    time_type m_timeout;
    time_type m_last_kicked;
};

#endif // _MESSAGE_WATCHDOG_INCLUDE_GUARD
