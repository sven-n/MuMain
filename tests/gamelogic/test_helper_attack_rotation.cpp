#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Skills/SkillManager.h"
#include "GameLogic/Automation/AttackSkillRotation.h"
#include "MUHelper/MuHelperData.h"
#include "MUHelper/MuHelper.h"
#include "Engine/Object/ZzzInterface.h"
#include <memory>

#include <array>
#include <cstring>

extern MovementSkill g_MovementSkill;

namespace
{
constexpr ActionSkillType Primary = static_cast<ActionSkillType>(41);
constexpr ActionSkillType ExtraOne = static_cast<ActionSkillType>(42);
constexpr ActionSkillType ExtraTwo = static_cast<ActionSkillType>(43);
using GameLogic::Automation::SkillResult;

struct SkillFixture
{
    CHARACTER hero;
    CHARACTER_MACHINE machine{};
    std::array<SKILL_ATTRIBUTE, MAX_SKILLS> attributes{};
    CHARACTER* previousHero = Hero;
    CHARACTER_MACHINE* previousMachine = CharacterMachine;
    CHARACTER_ATTRIBUTE* previousCharacter = CharacterAttribute;
    SKILL_ATTRIBUTE* previousAttributes = SkillAttribute;
    MUHelper::ConfigData config;
    GameLogic::Automation::AttackSkillRotation rotation;

    SkillFixture()
    {
        Hero = &hero;
        hero.Class = CLASS_KNIGHT;
        CharacterMachine = &machine;
        CharacterAttribute = &machine.Character;
        SkillAttribute = attributes.data();
        machine.Character.Mana = 100;
        machine.Character.SkillMana = 100;
        machine.Character.Strength = 500;
        machine.Character.Level = 100;
        machine.Character.SkillNumber = 3;
        config.aiSkill = {Primary, ExtraOne, ExtraTwo};
        for (std::size_t slot = 0; slot < config.aiSkill.size(); ++slot)
            machine.Character.Skill[slot] = static_cast<ActionSkillType>(config.aiSkill[slot]);
        gSkillManager.InvalidateSkillAttributeRequirementsCache();
    }

    ~SkillFixture()
    {
        Hero = previousHero;
        CharacterMachine = previousMachine;
        CharacterAttribute = previousCharacter;
        SkillAttribute = previousAttributes;
        gSkillManager.InvalidateSkillAttributeRequirementsCache();
    }

    ActionSkillType Select(int seconds = 0, std::size_t nearby = 0, std::size_t attacking = 0)
    {
        return rotation.Select(config, seconds, nearby, attacking);
    }
};
} // namespace

TEST_CASE("Unconditioned additional attacks rotate with the primary [helper_rotation]")
{
    SkillFixture fixture;
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        for (const auto expected : {Primary, ExtraOne, ExtraTwo})
        {
            CHECK(fixture.Select() == expected);
            fixture.rotation.Complete(SkillResult::Cast);
        }
    }
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    fixture.rotation.Reset();
    CHECK(fixture.Select() == Primary);
}

TEST_CASE("Automatic selection reads real cooldowns without consuming them [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Delay = 500;
    CharacterAttribute->SkillDelay[1] = 50;
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CHECK(CharacterAttribute->SkillDelay[1] == 50);
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraTwo);
    CHECK(CharacterAttribute->SkillDelay[1] == 50);

    CharacterAttribute->SkillDelay[1] = 0;
    fixture.rotation.Reset();
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CHECK(CharacterAttribute->SkillDelay[1] == 0);
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraOne);
    CHECK(CharacterAttribute->SkillDelay[1] == 0);

    SkillAttribute[AT_SKILL_NOVA].Delay = 500;
    CharacterAttribute->Skill[2] = AT_SKILL_NOVA;
    CharacterAttribute->SkillDelay[2] = 5;
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(AT_SKILL_NOVA_BEGIN));
    CharacterAttribute->SkillDelay[2] = 0;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(AT_SKILL_NOVA_BEGIN));
    CHECK(CharacterAttribute->SkillDelay[2] == 0);
}

TEST_CASE("Resource shortages reach execution while unlearned skills are skipped [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Mana = 10;
    SkillAttribute[ExtraOne].AbilityGuage = 5;
    CharacterAttribute->Mana = 9;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CharacterAttribute->Mana = 10;
    CharacterAttribute->SkillMana = 4;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CharacterAttribute->SkillMana = 5;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CharacterAttribute->Skill[1] = AT_SKILL_UNDEFINED;
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraTwo);
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(AT_SKILL_UNDEFINED));
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(static_cast<ActionSkillType>(MAX_SKILLS)));
}

TEST_CASE("Automatic eligibility retains attributes and skill-use conditions [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Strength = 501;
    gSkillManager.InvalidateSkillAttributeRequirementsCache();
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    SkillAttribute[ExtraOne].Strength = 500;
    gSkillManager.InvalidateSkillAttributeRequirementsCache();
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    SkillAttribute[ExtraOne].SkillUseType = SKILL_USE_TYPE_BRAND;
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
}

TEST_CASE("Busy and approaching actions keep their slot while cast and refusal advance [helper_rotation]")
{
    SkillFixture fixture;
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    for (const auto result : {SkillResult::Busy, SkillResult::Approaching})
    {
        CHECK(fixture.Select() == ExtraOne);
        fixture.rotation.Complete(result);
        CHECK(fixture.Select() == ExtraOne);
    }
    fixture.rotation.Complete(SkillResult::Refused);
    CHECK(fixture.Select() == ExtraTwo);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == Primary);
}

TEST_CASE("Timers and monster conditions keep priority and OR semantics [helper_rotation]")
{
    SkillFixture fixture;
    fixture.config.aiSkillCondition[1] = MUHelper::ON_TIMER;
    fixture.config.aiSkillInterval[1] = 5;
    CHECK(fixture.Select(0) == Primary);
    fixture.config.aiSkillInterval[1] = 0;
    CHECK(fixture.Select(5) == Primary);
    fixture.config.aiSkillInterval[1] = 5;
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select(1) == ExtraTwo);
    CHECK(fixture.Select(5) == ExtraOne);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select(6) == ExtraTwo);
    fixture.rotation.Reset();
    CHECK(fixture.Select(5) == ExtraOne);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select(6) == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    fixture.config.aiSkillCondition[1] |=
        MUHelper::ON_CONDITION | MUHelper::ON_MOBS_NEARBY | MUHelper::ON_MORE_THAN_TWO_MOBS;
    CHECK(fixture.Select(1, 2) == ExtraOne);
    CHECK(fixture.Select(5, 0) == ExtraOne);
    fixture.config.aiSkillCondition[1] =
        MUHelper::ON_CONDITION | MUHelper::ON_MOBS_ATTACKING | MUHelper::ON_MORE_THAN_THREE_MOBS;
    CHECK(fixture.Select(1, 5, 2) == ExtraTwo);
    CHECK(fixture.Select(1, 0, 3) == ExtraOne);
    fixture.config.aiSkillCondition[2] = MUHelper::ON_TIMER;
    fixture.config.aiSkillInterval[2] = 1;
    CHECK(fixture.Select(1, 0, 3) == ExtraOne);
    CHECK(fixture.Select(1, 0, 0) == ExtraTwo);
}

TEST_CASE("Primary fallback and primary-only behavior remain available [helper_rotation]")
{
    SkillFixture fixture;
    for (int slot = 0; slot < 3; ++slot)
        CharacterAttribute->SkillDelay[slot] = 100;
    CHECK(fixture.Select() == Primary);
    fixture.config.aiSkill = {Primary, 0, 0};
    CHECK(fixture.Select() == Primary);
    fixture.config.aiSkill = {0, 0, 0};
    CHECK(fixture.Select() == AT_SKILL_UNDEFINED);
}

TEST_CASE("Rotation is transient and preserves saved skill combo and pickup settings [helper_rotation]")
{
    SkillFixture fixture;
    fixture.config.bUseCombo = true;
    fixture.config.bPickAllItems = true;
    fixture.config.bPickSelectItems = true;
    fixture.config.aiSkillCondition[1] =
        MUHelper::ON_TIMER | MUHelper::ON_MOBS_NEARBY | MUHelper::ON_MORE_THAN_TWO_MOBS;
    fixture.config.aiSkillInterval[1] = 5;
    fixture.config.aiSkillCondition[2] =
        MUHelper::ON_CONDITION | MUHelper::ON_MOBS_NEARBY | MUHelper::ON_MORE_THAN_TWO_MOBS;
    PRECEIVE_MUHELPER_DATA before{};
    MUHelper::ConfigDataSerDe::Serialize(fixture.config, before);
    fixture.Select(5);
    fixture.rotation.Complete(SkillResult::Cast);
    PRECEIVE_MUHELPER_DATA after{};
    MUHelper::ConfigDataSerDe::Serialize(fixture.config, after);
    CHECK(std::memcmp(&before, &after, sizeof(before)) == 0);
    MUHelper::ConfigData restored;
    MUHelper::ConfigDataSerDe::Deserialize(after, restored);
    CHECK(restored.aiSkill == fixture.config.aiSkill);
    CHECK(restored.aiSkillCondition == fixture.config.aiSkillCondition);
    CHECK(restored.aiSkillInterval == fixture.config.aiSkillInterval);
    CHECK(restored.bUseCombo);
    CHECK(restored.bPickAllItems);
    CHECK(restored.bPickSelectItems);
}

TEST_CASE("Saved unconditioned slots rotate despite inactive condition subfields [helper_rotation]")
{
    SkillFixture fixture;
    PRECEIVE_MUHELPER_DATA packet{};
    MUHelper::ConfigDataSerDe::Serialize(fixture.config, packet);
    MUHelper::ConfigDataSerDe::Deserialize(packet, fixture.config);
    for (const auto expected : {Primary, ExtraOne, ExtraTwo})
    {
        CHECK(fixture.Select() == expected);
        fixture.rotation.Complete(SkillResult::Cast);
    }
}

TEST_CASE("Selected extra retains its actual slot after a skipped primary [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[Primary].Delay = 500;
    CharacterAttribute->SkillDelay[0] = 50;
    CHECK(fixture.Select() == ExtraOne);
    fixture.rotation.Complete(SkillResult::Approaching);
    CharacterAttribute->SkillDelay[0] = 0;
    CHECK(fixture.Select() == ExtraOne);
    fixture.rotation.Complete(SkillResult::Busy);
    CHECK(fixture.Select() == ExtraOne);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraTwo);
}

TEST_CASE("Real target failures preserve the next attack slot [helper_rotation]")
{
    SkillFixture fixture;
    auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    auto* previous = CharactersClient;
    CharactersClient = characters.get();
    auto& target = characters[0];
    target.Key = 123;
    target.Object.Live = true;
    target.Dead = 1;
    for (const auto expected : {Primary, ExtraOne, ExtraTwo})
    {
        CHECK(fixture.Select() == expected);
        const auto result = GameLogic::Automation::CastSkill(expected, true, target.Key, true, 6);
        CHECK(result == SkillResult::NotAttackable);
        fixture.rotation.Complete(result);
        CHECK(fixture.Select() == expected);
        target.Object.Live = false;
        const auto gone = GameLogic::Automation::CastSkill(expected, true, target.Key, true, 6);
        CHECK(gone == SkillResult::NotInView);
        fixture.rotation.Complete(gone);
        CHECK(fixture.Select() == expected);
        fixture.rotation.Complete(SkillResult::Cast);
        target.Object.Live = true;
    }
    CharactersClient = previous;
}

TEST_CASE("Long execution animations retain their selected turn [helper_rotation]")
{
    SkillFixture fixture;
    Hero->Object.Type = MODEL_PLAYER;
    for (const auto action : {PLAYER_SKILL_CHAIN_LIGHTNING, PLAYER_SKILL_THRUST, PLAYER_WALK_MALE})
    {
        fixture.rotation.Reset();
        Hero->Object.CurrentAction = action;
        for (int attempt = 0; attempt < 9; ++attempt)
        {
            REQUIRE(fixture.Select() == Primary);
            g_MovementSkill.m_iSkill = ExtraTwo;
            g_MovementSkill.m_bMagic = false;
            const auto result = GameLogic::Automation::CastSkill(Primary, false, -1, true, 6);
            CHECK(result == SkillResult::Busy);
            CHECK(g_MovementSkill.m_iSkill == ExtraTwo);
            CHECK_FALSE(g_MovementSkill.m_bMagic);
            fixture.rotation.Complete(result);
            CHECK(CharacterAttribute->SkillDelay[0] == 0);
        }
        CHECK(fixture.Select() == Primary);
    }
}

TEST_CASE("Ready skills that repeatedly produce no action yield a bounded turn [helper_rotation]")
{
    SkillFixture fixture;
    Hero->Object.Type = MODEL_PLAYER;
    Hero->Object.CurrentAction = PLAYER_STOP_MALE;
    for (int attempt = 0; attempt < 4; ++attempt)
    {
        REQUIRE(fixture.Select() == Primary);
        CharacterAttribute->Skill[0] = AT_SKILL_UNDEFINED;
        const auto result = GameLogic::Automation::CastSkill(Primary, false, -1, true, 6);
        CHECK(result == SkillResult::NotReady);
        fixture.rotation.Complete(result);
        CharacterAttribute->Skill[0] = Primary;
    }
    CHECK(fixture.Select() == ExtraOne);
}

TEST_CASE("Helper timer feeds real execution outcomes back into rotation [helper_rotation]")
{
    SkillFixture fixture;
    auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    auto* previous = CharactersClient;
    CharactersClient = characters.get();
    auto& target = characters[0];
    target.Key = 123;
    target.Object.Live = true;
    target.Object.Kind = KIND_MONSTER;
    target.PositionX = target.TargetX = Hero->PositionX = 100;
    target.PositionY = target.TargetY = Hero->PositionY = 100;
    target.Object.Position[0] = target.Object.Position[1] = 100 * TERRAIN_SCALE;
    Hero->Object.Position[0] = Hero->Object.Position[1] = 100 * TERRAIN_SCALE;
    Hero->Object.Type = MODEL_PLAYER;
    Hero->Object.CurrentAction = PLAYER_SKILL_CHAIN_LIGHTNING;
    fixture.config.iHuntingRange = 6;
    fixture.config.bFallbackBasicAttack = false;
    for (auto skill : fixture.config.aiSkill)
        SkillAttribute[skill].Distance = 6;
    MUHelper::g_MuHelper.Load(fixture.config);
    MUHelper::g_MuHelper.Start();
    MUHelper::g_MuHelper.AddTarget(target.Key, false);
    for (int attempt = 0; attempt < 9; ++attempt)
    {
        MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
        CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
        CHECK(CharacterAttribute->SkillDelay[0] == 0);
    }
    // The real admission check refuses insufficient AG and advances once.
    Hero->Object.CurrentAction = PLAYER_STOP_MALE;
    SkillAttribute[Primary].AbilityGuage = 101;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    Hero->Object.CurrentAction = PLAYER_SKILL_CHAIN_LIGHTNING;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == ExtraOne);
    SkillAttribute[Primary].AbilityGuage = 0;
    MUHelper::g_MuHelper.Load(fixture.config);
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
    auto& replacement = characters[1];
    replacement.Key = 124;
    replacement.Object.Live = true;
    replacement.Object.Kind = KIND_MONSTER;
    replacement.PositionX = replacement.TargetX = 100;
    replacement.PositionY = replacement.TargetY = 100;
    replacement.Object.Position[0] = replacement.Object.Position[1] = 100 * TERRAIN_SCALE;
    target.Dead = 1;
    MUHelper::g_MuHelper.AddTarget(replacement.Key, false);
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
    replacement.Object.Live = false;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    target.Dead = 0;
    MUHelper::g_MuHelper.AddTarget(target.Key, false);
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
    Hero->Object.CurrentAction = PLAYER_STOP_MALE;
    SkillAttribute[Primary].AbilityGuage = 101;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    Hero->Object.CurrentAction = PLAYER_SKILL_CHAIN_LIGHTNING;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == ExtraOne);
    SkillAttribute[Primary].AbilityGuage = 0;
    MUHelper::g_MuHelper.Stop();
    MUHelper::g_MuHelper.Start();
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
    MUHelper::g_MuHelper.Stop();
    MUHelper::g_MuHelper.DeleteAllTargets();
    CharactersClient = previous;
}

TEST_CASE("Unaffordable primary reaches the normal mana-potion path [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[Primary].Mana = 100;
    SkillAttribute[ExtraOne].Mana = 10;
    CharacterAttribute->Mana = 50;
    CHECK(fixture.Select() == Primary);
    CHECK(CharacterAttribute->Mana == 50);
    CHECK(CharacterAttribute->SkillDelay[0] == 0);
}

TEST_CASE("Zero-delay skills ignore stale delay counters as execution does [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Delay = 0;
    CharacterAttribute->SkillDelay[1] = 50;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CHECK(CharacterAttribute->SkillDelay[1] == 50);
}
