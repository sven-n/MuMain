#pragma once

#include <cstdint>

namespace Data::ServerList
{
// The load (in percent) from which a server is full, and overloaded.
inline constexpr int FullLoadPercent = 100;
inline constexpr int OverloadedLoadPercent = 128;

// A server's load as the login screen shows it (docs/server-list.md).
enum class LoadState : std::uint8_t
{
    // Below FullLoadPercent: the server can be joined, and {load} is empty.
    Open,
    Full,
    Overloaded,
};

// The state of the load the connect server sent.
LoadState GetLoadState(int percent);
} // namespace Data::ServerList
