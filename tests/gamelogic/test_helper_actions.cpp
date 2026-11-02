#include "stdafx.h"
#include "doctest.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Automation/Pickup.h"
#include "GameLogic/Automation/Skill.h"
#include "GameLogic/Skills/SkillManager.h"
#include "MUHelper/MuHelper.h"
#include "Render/Terrain/ZzzLodTerrain.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

namespace
{
constexpr ActionSkillType Primary = static_cast<ActionSkillType>(41);
constexpr ActionSkillType Extra = static_cast<ActionSkillType>(42);
constexpr int HeroTile = 103;
constexpr int HeroRow = 100;
constexpr int DropSlot = MAX_ITEMS - 1;
constexpr int OtherDropSlot = MAX_ITEMS - 2;
std::vector<std::pair<ActionSkillType, int>> submittedCasts;
std::vector<int> pickupAttempts;
GameLogic::Automation::PickupResult pickupResult;

struct HelperActionFixture
{
    CHARACTER hero;
    CHARACTER_MACHINE machine{};
    std::array<SKILL_ATTRIBUTE, MAX_SKILLS> attributes{};
    std::unique_ptr<CHARACTER[]> characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    CHARACTER* previousHero = Hero;
    CHARACTER* previousCharacters = CharactersClient;
    CHARACTER_MACHINE* previousMachine = CharacterMachine;
    CHARACTER_ATTRIBUTE* previousCharacter = CharacterAttribute;
    SKILL_ATTRIBUTE* previousAttributes = SkillAttribute;
    std::array<WORD, 9> previousWalls;
    std::array<bool, 2> previousDropLive = {Items[DropSlot].Object.Live, Items[OtherDropSlot].Object.Live};
    std::array<std::array<float, 3>, 2> previousDropPositions;

    HelperActionFixture()
    {
        submittedCasts.clear();
        pickupAttempts.clear();
        pickupResult = GameLogic::Automation::PickupResult::NoPath;
        for (int drop = 0; drop < previousDropLive.size(); ++drop)
            std::copy_n(Items[DropSlot - drop].Object.Position, 3, previousDropPositions[drop].begin());
        InitializeCharacter();
        MUHelper::ConfigData config;
        config.aiSkill = {Primary, Extra, 0};
        config.iHuntingRange = config.iObtainingRange = 8;
        config.bPickAllItems = true;
        config.bReturnToOriginalPosition = true;
        MUHelper::g_MuHelper.Load(config);
        MUHelper::g_MuHelper.Start();
        hero.PositionX = HeroTile;
        hero.Object.Position[0] = (HeroTile + 0.5f) * TERRAIN_SCALE;
        hero.Object.Position[1] = (HeroRow + 0.5f) * TERRAIN_SCALE;
        auto& target = characters[0];
        target.Key = 123;
        target.Object.Live = true;
        target.Object.Kind = KIND_MONSTER;
        target.PositionX = target.TargetX = HeroTile;
        target.PositionY = target.TargetY = HeroRow;
        target.Object.Position[0] = hero.Object.Position[0];
        target.Object.Position[1] = hero.Object.Position[1];
        MUHelper::g_MuHelper.AddTarget(target.Key, false);
        PlaceDrop(DropSlot, HeroTile + 3);
        BlockReturnPath();
    }

    void InitializeCharacter()
    {
        Hero = &hero;
        CharactersClient = characters.get();
        CharacterMachine = &machine;
        CharacterAttribute = &machine.Character;
        SkillAttribute = attributes.data();
        hero.Class = CLASS_KNIGHT;
        hero.Object.Type = MODEL_PLAYER;
        hero.Object.CurrentAction = PLAYER_STOP_MALE;
        hero.PositionX = HeroTile - 3;
        hero.PositionY = HeroRow;
        machine.Character.Mana = machine.Character.SkillMana = 100;
        machine.Character.Strength = 500;
        machine.Character.Level = 100;
        machine.Character.SkillNumber = 2;
        machine.Character.Skill[0] = Primary;
        machine.Character.Skill[1] = Extra;
        attributes[Primary].Distance = attributes[Extra].Distance = 8;
        gSkillManager.InvalidateSkillAttributeRequirementsCache();
    }

    void BlockReturnPath()
    {
        int wall = 0;
        for (int y = -1; y <= 1; ++y)
            for (int x = -1; x <= 1; ++x)
            {
                auto& terrain = TerrainWall[TERRAIN_INDEX(HeroTile + x, HeroRow + y)];
                previousWalls[wall++] = terrain;
                terrain = x == 0 && y == 0 ? 0 : TW_NOMOVE;
            }
        InitPath();
    }

    ~HelperActionFixture()
    {
        MUHelper::g_MuHelper.Stop();
        MUHelper::g_MuHelper.DeleteAllTargets();
        MUHelper::g_MuHelper.DeleteItem(DropSlot);
        MUHelper::g_MuHelper.DeleteItem(OtherDropSlot);
        for (int drop = 0; drop < previousDropLive.size(); ++drop)
        {
            auto& object = Items[DropSlot - drop].Object;
            object.Live = previousDropLive[drop];
            std::copy(previousDropPositions[drop].begin(), previousDropPositions[drop].end(), object.Position);
        }
        int wall = 0;
        for (int y = -1; y <= 1; ++y)
            for (int x = -1; x <= 1; ++x)
                TerrainWall[TERRAIN_INDEX(HeroTile + x, HeroRow + y)] = previousWalls[wall++];
        Hero = previousHero;
        CharactersClient = previousCharacters;
        CharacterMachine = previousMachine;
        CharacterAttribute = previousCharacter;
        SkillAttribute = previousAttributes;
        gSkillManager.InvalidateSkillAttributeRequirementsCache();
    }

    void PlaceDrop(int slot, int tileX)
    {
        Items[slot].Object.Live = true;
        Items[slot].Object.Position[0] = (tileX + 0.5f) * TERRAIN_SCALE;
        Items[slot].Object.Position[1] = (HeroRow + 0.5f) * TERRAIN_SCALE;
        MUHelper::g_MuHelper.AddItem(slot, {tileX, HeroRow});
    }

    void Tick()
    {
        MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    }
};
} // namespace

// Link-time action spies verify helper submissions without a game server; the real primitives have separate tests.
namespace GameLogic::Automation
{
SkillResult CastSkill(ActionSkillType skill, bool, int targetKey, bool, int)
{
    submittedCasts.emplace_back(skill, targetKey);
    return SkillResult::Cast;
}

bool IsSelfPositionSkill(ActionSkillType)
{
    return false;
}

PickupResult PickUpItem(int slot, int)
{
    pickupAttempts.push_back(slot);
    return pickupResult;
}
} // namespace GameLogic::Automation

TEST_CASE("Blocked loot and return paths still submit successful cast actions [helper_actions]")
{
    HelperActionFixture fixture;
    constexpr int TicksThroughReturn = 6;
    for (int tick = 0; tick < TicksThroughReturn; ++tick)
        fixture.Tick();
    REQUIRE(submittedCasts.size() == TicksThroughReturn);
    for (int tick = 0; tick < TicksThroughReturn; ++tick)
    {
        CHECK(submittedCasts[tick].first == (tick % 2 == 0 ? Primary : Extra));
        CHECK(submittedCasts[tick].second == 123);
    }
    CHECK(pickupAttempts == std::vector<int>{DropSlot});
}

TEST_CASE("Blocked drops retry after cooldown while other loot and casts continue [helper_actions]")
{
    HelperActionFixture fixture;
    fixture.Tick();
    REQUIRE(pickupAttempts == std::vector<int>{DropSlot});
    fixture.PlaceDrop(OtherDropSlot, HeroTile + 4);
    pickupResult = GameLogic::Automation::PickupResult::Requested;
    fixture.Tick();
    CHECK(pickupAttempts == std::vector<int>{DropSlot, OtherDropSlot});
    CHECK(submittedCasts.size() == 2);
    constexpr auto AfterPickupRetry = std::chrono::milliseconds(1100);
    std::this_thread::sleep_for(AfterPickupRetry);
    fixture.Tick();
    CHECK(pickupAttempts == std::vector<int>{DropSlot, OtherDropSlot, DropSlot});
    CHECK(submittedCasts.size() == 3);
}

TEST_CASE("A reused drop slot does not inherit a failed pickup cooldown [helper_actions]")
{
    HelperActionFixture fixture;
    fixture.Tick();
    MUHelper::g_MuHelper.DeleteItem(DropSlot);
    fixture.PlaceDrop(DropSlot, HeroTile + 2);
    fixture.Tick();
    CHECK(pickupAttempts == std::vector<int>{DropSlot, DropSlot});
    CHECK(submittedCasts.size() == 2);
}
