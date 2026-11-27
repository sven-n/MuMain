#include "stdafx.h"

// A monster whose files arrive after its spawn (GameLogic/Monsters/MonsterPacks.h): CreateMonster makes a stand-in, and
// the frame's pump makes the monster again, in place, when the files are there.

#include "MonsterCreationScope.h"

#include "Core/Content/ContentStore.h"
#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Monsters/MonsterPacks.h"

#include <doctest.h>

#include <string>
#include <string_view>

namespace
{
using State = Content::PackStatus::State;

// A target's pack source for the test: the packs it answers for are in `state`, the others are ready.
struct FakePacks
{
    FakePacks(std::string pack, State state) : pack(std::move(pack)), state(state)
    {
        Content::SetPackSource(
            [this](std::string_view asked)
            { return Content::PackStatus{.state = asked == this->pack ? this->state : State::Ready}; });
    }
    ~FakePacks()
    {
        Content::SetPackSource({});
        GameLogic::Monsters::Packs::Cancel();
    }

    std::string pack;
    State state;
};
} // namespace

TEST_CASE("A monster whose pack is pending stands in, then takes its own model when the pack is ready "
          "[monsters][packs]")
{
    MonsterRecorder::CreationScope scope(0);
    constexpr int number = MONSTER_BULL_FIGHTER + 1;
    const int ownModel = scope.CreatedModel(number);
    REQUIRE(ownModel != MODEL_BULL_FIGHTER);

    FakePacks packs(GameLogic::Monsters::Packs::PackOf(number), State::Pending);
    CHARACTER* monster = scope.Create(number);
    REQUIRE(monster != nullptr);
    CHECK(monster->Object.Type == MODEL_BULL_FIGHTER);
    CHECK(monster->MonsterIndex == number);
    const int key = monster->Key;
    const int x = monster->PositionX;
    const int y = monster->PositionY;

    // Still pending: the stand-in stays.
    RemakeMonstersWhoseFilesArrived();
    CHECK(FindCharacterByKey(key)->Object.Type == MODEL_BULL_FIGHTER);

    packs.state = State::Ready;
    RemakeMonstersWhoseFilesArrived();
    CHARACTER* remade = FindCharacterByKey(key);
    REQUIRE(remade != nullptr);
    CHECK(remade == monster);
    CHECK(remade->Object.Type == ownModel);
    CHECK(remade->MonsterIndex == number);
    CHECK(remade->PositionX == x);
    CHECK(remade->PositionY == y);
}

TEST_CASE("A monster whose pack failed is made again as the unknown monsters' look allows [monsters][packs]")
{
    MonsterRecorder::CreationScope scope(0);
    constexpr int number = MONSTER_BULL_FIGHTER + 1;

    FakePacks packs(GameLogic::Monsters::Packs::PackOf(number), State::Pending);
    CHARACTER* monster = scope.Create(number);
    REQUIRE(monster != nullptr);
    const int key = monster->Key;

    packs.state = State::Failed;
    RemakeMonstersWhoseFilesArrived();
    // Made again (here the files exist, so with its own model), not left as the stand-in; nothing waits any more.
    CHECK(GameLogic::Monsters::Packs::WaitingCount() == 0);
    REQUIRE(FindCharacterByKey(key) != nullptr);
    CHECK(FindCharacterByKey(key)->MonsterIndex == number);
}

TEST_CASE("A stand-in whose slot holds another monster by then is not remade [monsters][packs]")
{
    MonsterRecorder::CreationScope scope(0);
    constexpr int number = MONSTER_BULL_FIGHTER + 1;
    FakePacks packs(GameLogic::Monsters::Packs::PackOf(number), State::Pending);
    CHARACTER* monster = scope.Create(number);
    REQUIRE(monster != nullptr);

    monster->MonsterIndex = MONSTER_BULL_FIGHTER;
    packs.state = State::Ready;
    RemakeMonstersWhoseFilesArrived();
    CHECK(monster->Object.Type == MODEL_BULL_FIGHTER);
    CHECK(GameLogic::Monsters::Packs::WaitingCount() == 0);
}
