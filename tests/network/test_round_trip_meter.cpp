#include "doctest.h"

#include "Network/RoundTripMeter.h"

using Network::RoundTripMeter;
using namespace std::chrono_literals;

TEST_CASE("the ping asks right away and then once a second while it is shown [network][ping]")
{
    RoundTripMeter meter;
    const RoundTripMeter::Clock::time_point start{};
    CHECK_FALSE(meter.NextRequest(start).has_value());

    meter.Show(start);
    const auto first = meter.NextRequest(start);
    REQUIRE(first.has_value());
    CHECK_FALSE(meter.NextRequest(start + 999ms).has_value());
    const auto second = meter.NextRequest(start + 1s);
    REQUIRE(second.has_value());
    CHECK(*second != *first);
    CHECK_FALSE(meter.NextRequest(start + RoundTripMeter::ShowTime).has_value());
    CHECK_FALSE(meter.Read(start + RoundTripMeter::ShowTime).shown);
}

TEST_CASE("the ping is the time from a request to its answer [network][ping]")
{
    RoundTripMeter meter;
    const RoundTripMeter::Clock::time_point start{};
    meter.Show(start);
    const uint32_t first = *meter.NextRequest(start);
    const uint32_t second = *meter.NextRequest(start + 1s);

    auto reading = meter.Read(start + 10ms);
    CHECK(reading.shown);
    CHECK_FALSE(reading.milliseconds.has_value());
    CHECK_FALSE(reading.unanswered);

    meter.Answered(second, start + 1s + 42ms);
    CHECK(meter.Read(start + 1s + 50ms).milliseconds == 42);

    // A late answer to an older request still counts once; unknown or repeated tokens don't.
    meter.Answered(first, start + 1s + 60ms);
    CHECK(meter.Read(start + 1s + 70ms).milliseconds == 1060);
    meter.Answered(first, start + 2s);
    meter.Answered(12345, start + 2s);
    CHECK(meter.Read(start + 2s).milliseconds == 1060);
}

TEST_CASE("a server which doesn't answer reads as unanswered, and $ping again starts over [network][ping]")
{
    RoundTripMeter meter;
    const RoundTripMeter::Clock::time_point start{};
    meter.Show(start);
    const uint32_t token = *meter.NextRequest(start);
    CHECK_FALSE(meter.Read(start + RoundTripMeter::AnswerTimeout - 1ms).unanswered);
    CHECK(meter.Read(start + RoundTripMeter::AnswerTimeout).unanswered);

    meter.Answered(token, start + 1s);
    const auto later = start + 10s;
    meter.Show(later);
    const auto reading = meter.Read(later);
    CHECK(reading.shown);
    CHECK_FALSE(reading.milliseconds.has_value());
    CHECK_FALSE(reading.unanswered);
    // The request of the earlier showing is forgotten.
    meter.Answered(token, later + 5ms);
    CHECK_FALSE(meter.Read(later + 5ms).milliseconds.has_value());
}
