#include "stdafx.h"
#include "doctest.h"

#include "Core/Utilities/DrawSpawnCheck.h"
#include "Core/Utilities/Random.h"
#include "Engine/Object/ZzzObject.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Render/Items/ItemEffects.h"
#include "Render/Models/ZzzBMD.h"
#include "ShippedLooksScope.h"
#include "../data/ItemModelDatabaseScope.h"

#include <functional>
#include <latch>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <execinfo.h>
#endif

namespace
{
using namespace Diagnostics::DrawSpawnCheck;

#if defined(_MSC_VER)
#define MU_NOINLINE __declspec(noinline)
#else
#define MU_NOINLINE __attribute__((noinline))
#endif

// An address inside the calling function; it reads the frame after the capture, so the capture is no tail call.
MU_NOINLINE void* CallerOfHere()
{
    void* frames[2] = {};
#ifdef _WIN32
    const int count = CaptureStackBackTrace(0, 2, frames, nullptr);
#else
    const int count = backtrace(frames, 2);
#endif
    return count == 2 ? frames[1] : nullptr;
}

// Two draws that draw from the update's sequences; the guard should name each. `here` gets an address inside them.
MU_NOINLINE int DrawsFromTheUpdateSequence(void** here)
{
    const int value = Random::Rand();
    *here = CallerOfHere();
    return value + 1;
}

MU_NOINLINE float DrawsFromTheEngine(void** here)
{
    const float value = Random::Unit();
    *here = CallerOfHere();
    return value + 1.f;
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

TEST_CASE("The guard names each function that drew from a gameplay sequence while drawing [random]")
{
    const LegacyStateScope legacy;
    const auto drawnFrom = [](auto draw, const char* kind)
    {
        CAPTURE(kind);
        ResetViolations();
        void* here = nullptr;
        {
            const DrawScope drawing;
            draw(&here);
        }
        REQUIRE(Violations() == 1);
        const Violation violation = LastViolation();
        CHECK(violation.kind == kind);
        CAPTURE(violation.caller);
        // The recorded return address lies in the drawing function, a few instructions from the address it took itself.
        REQUIRE(here != nullptr);
        REQUIRE(violation.callerAddress != nullptr);
        const auto distance = static_cast<char*>(violation.callerAddress) - static_cast<char*>(here);
        CHECK(distance > -256);
        CHECK(distance < 256);
        CHECK(!violation.caller.empty());
        return violation.callerAddress;
    };
    void* const legacySite = drawnFrom(DrawsFromTheUpdateSequence, "legacy random draw");
    void* const engineSite = drawnFrom(DrawsFromTheEngine, "random engine draw");
    // Two sites, two callers: the log names each of them once.
    CHECK(legacySite != engineSite);
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

TEST_CASE("Threads drawing at the same time each count every call [random]")
{
    constexpr int threads = 8;
    constexpr int callsEach = 2000;
    ResetViolations();
    std::latch start(threads);
    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t)
        workers.emplace_back(
            [&start]
            {
                const DrawScope drawing;
                start.arrive_and_wait();
                for (int i = 0; i < callsEach; ++i)
                    Random::Unit();
            });
    for (std::thread& worker : workers)
        worker.join();
    CHECK(Violations() == threads * callsEach);
    ResetViolations();
}

TEST_CASE("Setting up a shimmering item for a draw takes nothing from the gameplay sequences [random]")
{
    // The shimmer is the item effect's (design L27): its random scroll is drawn from the draw's own randomness.
    const ShippedLooksScope looks;
    REQUIRE(looks.IsComplete());
    const ItemModelDatabaseScope restoreModels;
    const auto model = [](int group, int number, const char* effect)
    {
        Data::Items::ItemModelDefinition entry;
        entry.group = group;
        entry.number = number;
        entry.file = "Data/Item/Sword01.bmd";
        entry.itemEffect = effect;
        return entry;
    };
    g_ItemModelDatabase.Build(std::vector<Data::Items::ItemModelDefinition>{model(6, 14, "shimmeringGlow"),
                                                                            model(5, 5, "shimmeringGlow"),
                                                                            model(5, 7, "flickeringGlow"),
                                                                            model(0, 14, "shimmeringPulse")},
                              Data::Effects::GlowColorList{});
    const LegacyStateScope legacy;
    const std::uint32_t legacyBefore = Random::RandState();
    const std::mt19937 engineBefore = Random::Snapshot();
    ResetViolations();
    for (const int type : {static_cast<int>(MODEL_LEGENDARY_SHIELD), static_cast<int>(MODEL_LEGENDARY_STAFF),
                           static_cast<int>(MODEL_CHAOS_LIGHTNING_STAFF), static_cast<int>(MODEL_LIGHTING_SWORD)})
    {
        CAPTURE(type);
        OBJECT item;
        item.Type = type;
        DrawLook look = DrawLook::Of(item);
        float alpha = 1.f;
        int level = 0;
        {
            // The inventory view and a held part set the item up like this in every pass.
            const DrawScope drawing;
            ItemObjectAttribute(&item);
            // A model of its own: this program opens no model slots.
            BMD model;
            Render::Items::ItemEffects::Apply(&model, &item, look, type, alpha, level);
        }
        CHECK(look.blendMesh >= 1);
    }
    CHECK(Violations() == 0);
    CHECK(Random::RandState() == legacyBefore);
    CHECK(Random::Snapshot() == engineBefore);
}
