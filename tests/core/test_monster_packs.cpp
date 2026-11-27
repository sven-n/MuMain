#include <doctest.h>

#include "Core/Content/ContentStore.h"
#include "GameLogic/Monsters/MonsterPacks.h"

#include <string>
#include <string_view>
#include <vector>

namespace
{
namespace Packs = GameLogic::Monsters::Packs;
using State = Content::PackStatus::State;

// A target's pack source for the test: answers `state` for every pack and records the asks.
struct FakePacks
{
    explicit FakePacks(State state) : state(state)
    {
        Content::SetPackSource(
            [this](std::string_view pack)
            {
                asked.emplace_back(pack);
                return Content::PackStatus{.state = this->state, .progress = 0.5f, .error = "HTTP 404"};
            });
    }
    ~FakePacks()
    {
        Content::SetPackSource({});
        Packs::Cancel();
    }

    State state;
    std::vector<std::string> asked;
};

// What the game does with a monster whose pack is ready or failed: makes it again.
struct Remade
{
    std::vector<int> keys;
    auto Callback()
    {
        return [this](const Packs::Waiting& monster) { keys.push_back(monster.key); };
    }
};
} // namespace

TEST_CASE("a monster's pack is named after its number [monster packs]")
{
    CHECK(Packs::PackOf(7) == "monster7");
    CHECK(Packs::PackOf(412) == "monster412");
}

TEST_CASE("without a pack source every monster is made at once [monster packs]")
{
    CHECK(Packs::ReadyToCreate(7, 100));
    CHECK(Packs::WaitingCount() == 0);
}

TEST_CASE("a ready pack makes the monster at once and asks for its pack [monster packs]")
{
    FakePacks packs(State::Ready);
    CHECK(Packs::ReadyToCreate(7, 100));
    CHECK(Packs::WaitingCount() == 0);
    REQUIRE(packs.asked.size() == 1);
    CHECK(packs.asked[0] == "monster7");
}

TEST_CASE("a slow pack keeps the monster waiting, then makes it once when ready [monster packs]")
{
    FakePacks packs(State::Pending);
    Remade remade;

    CHECK_FALSE(Packs::ReadyToCreate(7, 100));
    CHECK(Packs::WaitingCount() == 1);

    Packs::Pump(remade.Callback());
    Packs::Pump(remade.Callback());
    CHECK(remade.keys.empty());
    CHECK(Packs::WaitingCount() == 1);

    packs.state = State::Ready;
    Packs::Pump(remade.Callback());
    CHECK(remade.keys == std::vector<int>{100});
    CHECK(Packs::WaitingCount() == 0);

    Packs::Pump(remade.Callback());
    CHECK(remade.keys.size() == 1);
}

TEST_CASE("a failed pack makes the monster at once, and a pack that fails while waiting makes it too [monster packs]")
{
    FakePacks packs(State::Failed);
    Remade remade;

    // Made now, as the unknown monsters' look for its missing file.
    CHECK(Packs::ReadyToCreate(7, 100));
    CHECK(Packs::WaitingCount() == 0);

    packs.state = State::Pending;
    CHECK_FALSE(Packs::ReadyToCreate(8, 101));
    packs.state = State::Failed;
    Packs::Pump(remade.Callback());
    CHECK(remade.keys == std::vector<int>{101});
    CHECK(Packs::WaitingCount() == 0);
}

TEST_CASE("each waiting monster follows its own pack, and the same key replaces its entry [monster packs]")
{
    FakePacks packs(State::Pending);
    Remade remade;
    CHECK_FALSE(Packs::ReadyToCreate(7, 100));
    CHECK_FALSE(Packs::ReadyToCreate(8, 101));
    CHECK_FALSE(Packs::ReadyToCreate(9, 100));
    CHECK(Packs::WaitingCount() == 2);

    Content::SetPackSource(
        [](std::string_view pack)
        { return Content::PackStatus{.state = pack == "monster8" ? State::Ready : State::Pending}; });
    Packs::Pump(remade.Callback());
    CHECK(remade.keys == std::vector<int>{101});
    CHECK(Packs::WaitingCount() == 1);
}

TEST_CASE("a torn down session drops the waiting monsters [monster packs]")
{
    FakePacks packs(State::Pending);
    Remade remade;
    CHECK_FALSE(Packs::ReadyToCreate(7, 100));
    Packs::Cancel();
    packs.state = State::Ready;
    Packs::Pump(remade.Callback());
    CHECK(remade.keys.empty());
    CHECK(Packs::WaitingCount() == 0);
}
