#include "doctest.h"

#include "Network/ConnectionLifecycle.h"

using Network::ConnectionEvent;
using Network::ConnectionLifecycle;
using Network::ConnectionState;

TEST_CASE("A connection is connecting until the network says otherwise [network]")
{
    ConnectionLifecycle lifecycle;
    CHECK(lifecycle.State() == ConnectionState::Connecting);
    CHECK_FALSE(lifecycle.FailedToOpen());
    lifecycle.Apply(ConnectionEvent::None);
    CHECK(lifecycle.State() == ConnectionState::Connecting);
}

TEST_CASE("Opened then closed is a lost connection, not a failed one [network]")
{
    ConnectionLifecycle lifecycle;
    lifecycle.Apply(ConnectionEvent::Opened);
    CHECK(lifecycle.State() == ConnectionState::Open);
    lifecycle.Apply(ConnectionEvent::Closed);
    CHECK(lifecycle.State() == ConnectionState::Closed);
    CHECK_FALSE(lifecycle.FailedToOpen());
}

TEST_CASE("Closed before opening is a failed connect [network]")
{
    ConnectionLifecycle lifecycle;
    lifecycle.Apply(ConnectionEvent::Closed);
    CHECK(lifecycle.State() == ConnectionState::Closed);
    CHECK(lifecycle.FailedToOpen());
}

TEST_CASE("A close is final [network]")
{
    ConnectionLifecycle lifecycle;
    lifecycle.Apply(ConnectionEvent::Closed);
    lifecycle.Apply(ConnectionEvent::Opened);
    CHECK(lifecycle.State() == ConnectionState::Closed);
    CHECK(lifecycle.FailedToOpen());
}

TEST_CASE("The client closing a connect it started is no failed connect [network]")
{
    ConnectionLifecycle lifecycle;
    lifecycle.CloseLocally();
    CHECK(lifecycle.State() == ConnectionState::Closed);
    CHECK_FALSE(lifecycle.FailedToOpen());
    lifecycle.Apply(ConnectionEvent::Closed);
    CHECK_FALSE(lifecycle.FailedToOpen());
}

TEST_CASE("The client's own close counts at once [network]")
{
    ConnectionLifecycle lifecycle;
    lifecycle.Apply(ConnectionEvent::Opened);
    lifecycle.CloseLocally();
    CHECK(lifecycle.State() == ConnectionState::Closed);
    lifecycle.Apply(ConnectionEvent::Opened);
    CHECK(lifecycle.State() == ConnectionState::Closed);
    CHECK_FALSE(lifecycle.FailedToOpen());
}
