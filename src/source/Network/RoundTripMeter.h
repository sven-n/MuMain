#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>

namespace Network
{
// The ping shown after $ping: round trip requests to the game server, timed until the server answers each.
class RoundTripMeter
{
public:
    using Clock = std::chrono::steady_clock;

    // How long the ping stays on screen, how often it asks meanwhile, and how long it waits for the first answer
    // before saying the server gives none (a server without round trip requests ignores them).
    static constexpr auto ShowTime = std::chrono::seconds(5);
    static constexpr auto RequestInterval = std::chrono::seconds(1);
    static constexpr auto AnswerTimeout = std::chrono::seconds(3);

    struct Reading
    {
        bool shown = false;
        // The latest round trip; none while the first answer is outstanding.
        std::optional<int> milliseconds;
        // No answer to the first request within AnswerTimeout.
        bool unanswered = false;
    };

    // Shows the ping for ShowTime from now and asks right away; a new measurement, not the last one's value.
    void Show(Clock::time_point now);
    // The token of the request to send now, if one is due; the caller sends it.
    std::optional<uint32_t> NextRequest(Clock::time_point now);
    // An answer of the server; tokens it didn't ask for, or asked for too long ago, are ignored.
    void Answered(uint32_t token, Clock::time_point now);
    Reading Read(Clock::time_point now) const;

private:
    struct Request
    {
        uint32_t token = 0;
        Clock::time_point sent{};
    };

    Clock::time_point m_shownSince{};
    Clock::time_point m_shownUntil{};
    Clock::time_point m_nextRequest{};
    uint32_t m_nextToken = 1;
    // The requests still waiting, oldest overwritten: a few seconds' worth at RequestInterval.
    std::array<Request, 8> m_outstanding{};
    size_t m_nextSlot = 0;
    std::optional<Clock::duration> m_latest;
};

// The client's meter.
RoundTripMeter& Ping();
} // namespace Network
