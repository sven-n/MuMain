#pragma once

#include <cstdint>

namespace Network
{
// What the network tells the game about a connection, in order with its packets.
enum class ConnectionEvent : std::uint8_t
{
    None, // a packet, not an event
    Opened,
    Closed,
};

enum class ConnectionState : std::uint8_t
{
    Connecting,
    Open,
    Closed,
};

// A connection's state as the game thread sees it: connecting until the network says it opened or closed. A close
// is final; a close the client makes itself counts at once.
class ConnectionLifecycle
{
public:
    void Apply(ConnectionEvent event)
    {
        if (m_state == ConnectionState::Closed)
        {
            return;
        }

        if (event == ConnectionEvent::Opened)
        {
            m_state = ConnectionState::Open;
            m_opened = true;
        }
        else if (event == ConnectionEvent::Closed)
        {
            m_state = ConnectionState::Closed;
        }
    }

    void CloseLocally()
    {
        if (m_state != ConnectionState::Closed)
        {
            m_closedLocally = true;
        }
        m_state = ConnectionState::Closed;
    }

    ConnectionState State() const
    {
        return m_state;
    }

    // The network closed it before it ever opened: the server couldn't be reached. Not a close the client made.
    bool FailedToOpen() const
    {
        return m_state == ConnectionState::Closed && !m_opened && !m_closedLocally;
    }

private:
    ConnectionState m_state = ConnectionState::Connecting;
    bool m_opened = false;
    bool m_closedLocally = false;
};
} // namespace Network
