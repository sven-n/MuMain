#include "stdafx.h"
#include "doctest.h"

#include "Core/Utilities/DrawSpawnCheck.h"
#include "Core/Utilities/Random.h"

#include <functional>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
using namespace Diagnostics::DrawSpawnCheck;

#if defined(_MSC_VER)
#define MU_NOINLINE __declspec(noinline)
#else
#define MU_NOINLINE __attribute__((noinline))
#endif

// A draw that draws from the update's legacy sequence; the guard should name it. `here` gets an address inside it.
MU_NOINLINE int DrawsFromTheUpdateSequence(void** here)
{
    const int value = Random::Rand();
#ifdef _WIN32
    CaptureStackBackTrace(0, 1, here, nullptr);
#endif
    return value + 1;
}

// Every call that advances or reseeds one of the two gameplay sequences, with the kind the guard records.
std::vector<std::pair<std::string, std::function<void()>>> GameplayCalls()
{
    return {
        {"legacy random draw", [] { Random::Rand(); }},
        {"legacy random reseed", [] { Random::SeedRand(5); }},
        {"random engine draw", [] { Random::RangeInt(0, 9); }},
        {"random engine draw", [] { Random::RangeFloat(0.f, 1.f); }},
        {"random engine draw", [] { Random::Unit(); }},
        {"random engine draw", [] { Random::UnitDouble(); }},
        {"random engine draw", [] { Random::FpsCheck(2, 1.0); }},
        {"random engine draw", [] { Random::FpsCheck(2); }},
        {"random engine reseed", [] { Random::Seed(5); }},
    };
}

// Keeps the calling thread's legacy sequence where it was; the engine is reseeded by the calls themselves.
struct LegacyStateScope
{
    std::uint32_t state = Random::RandState();
    ~LegacyStateScope()
    {
        Random::SeedRand(state);
    }
};
} // namespace

TEST_CASE("A draw that advances or reseeds a gameplay random sequence is a violation, each call once [random]")
{
    const LegacyStateScope legacy;
    for (const auto& [kind, call] : GameplayCalls())
    {
        CAPTURE(kind);
        ResetViolations();
        {
            const DrawScope drawing;
            call();
        }
        CHECK(Violations() == 1);
        CHECK(LastViolation().kind == kind);
    }
    ResetViolations();
}

TEST_CASE("The update advances and reseeds its random sequences freely [random]")
{
    const LegacyStateScope legacy;
    ResetViolations();
    for (const auto& [kind, call] : GameplayCalls())
        call();
    // Bounds that leave nothing to draw change nothing, in a draw too.
    {
        const DrawScope drawing;
        Random::RangeInt(4, 4);
        Random::RangeFloat(1.f, 1.f);
        Random::FpsCheck(0, 1.0);
    }
    CHECK(Violations() == 0);
}

TEST_CASE("The guard names the function that drew from a gameplay sequence while drawing [random]")
{
    const LegacyStateScope legacy;
    ResetViolations();
    void* here = nullptr;
    {
        const DrawScope drawing;
        DrawsFromTheUpdateSequence(&here);
    }
    REQUIRE(Violations() == 1);
    const Violation violation = LastViolation();
    CHECK(violation.kind == "legacy random draw");
#ifdef _WIN32
    // The recorded return address lies in the drawing function, a few instructions from the address it took itself.
    CAPTURE(violation.caller);
    REQUIRE(violation.callerAddress != nullptr);
    const auto distance = static_cast<char*>(violation.callerAddress) - static_cast<char*>(here);
    CHECK(distance > -256);
    CHECK(distance < 256);
    CHECK(!violation.caller.empty());
#endif
    ResetViolations();
}

TEST_CASE("A draw's own randomness and reading the sequences leave both gameplay sequences as they were [random]")
{
    const LegacyStateScope legacy;
    Random::SeedRand(1234);
    Random::Seed(77);
    const std::uint32_t legacyBefore = Random::RandState();
    const std::mt19937 engineBefore = Random::Snapshot();
    int key = 0;

    ResetViolations();
    {
        const DrawScope drawing;
        for (std::uint32_t salt = 0; salt < 50; ++salt)
        {
            Random::DrawRand(&key, salt);
            Random::DrawFpsCheck(3, 1.0, &key, salt);
        }
        Random::RandState();
        Random::Snapshot();
    }
    CHECK(Violations() == 0);
    CHECK(Random::RandState() == legacyBefore);
    CHECK(Random::Snapshot() == engineBefore);

    // The next values are the ones the update would have drawn without the draw.
    std::mt19937 control = engineBefore;
    std::uniform_int_distribution<std::int32_t> pick(0, 1000);
    CHECK(Random::RangeInt(0, 1000) == pick(control));
    CHECK(Random::Rand() == static_cast<std::int32_t>(((legacyBefore * 214013u + 2531011u) >> 16) & 0x7FFF));
}

TEST_CASE("Another thread's random sequences are its own while the frame draws [random]")
{
    ResetViolations();
    {
        const DrawScope drawing;
        std::thread worker(
            []
            {
                Random::Rand();
                Random::SeedRand(9);
                Random::Unit();
                Random::Seed(9);
            });
        worker.join();
    }
    CHECK(Violations() == 0);
}
