#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Skills/SkillManager.h"
#include "MUHelper/AttackSkillRotation.h"
#include "MUHelper/MuHelperData.h"

#include <array>
#include <cstring>

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
    MUHelper::AttackSkillRotation rotation;

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
    CHECK_FALSE(MUHelper::IsAttackSkillReady(ExtraOne));
    CHECK(CharacterAttribute->SkillDelay[1] == 50);
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraTwo);
    CHECK(CharacterAttribute->SkillDelay[1] == 50);

    CharacterAttribute->SkillDelay[1] = 0;
    fixture.rotation.Reset();
    CHECK(MUHelper::IsAttackSkillReady(ExtraOne));
    CHECK(CharacterAttribute->SkillDelay[1] == 0);
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraOne);
    CHECK(CharacterAttribute->SkillDelay[1] == 0);

    CharacterAttribute->Skill[2] = AT_SKILL_NOVA;
    CharacterAttribute->SkillDelay[2] = 5;
    CHECK_FALSE(MUHelper::IsAttackSkillReady(AT_SKILL_NOVA_BEGIN));
    CharacterAttribute->SkillDelay[2] = 0;
    CHECK(MUHelper::IsAttackSkillReady(AT_SKILL_NOVA_BEGIN));
    CHECK(CharacterAttribute->SkillDelay[2] == 0);
}

TEST_CASE("Automatic attacks skip unlearned and resource-ineligible skills [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Mana = 10;
    SkillAttribute[ExtraOne].AbilityGuage = 5;
    CharacterAttribute->Mana = 9;
    CHECK_FALSE(MUHelper::IsAttackSkillReady(ExtraOne));
    CharacterAttribute->Mana = 10;
    CharacterAttribute->SkillMana = 4;
    CHECK_FALSE(MUHelper::IsAttackSkillReady(ExtraOne));
    CharacterAttribute->SkillMana = 5;
    CHECK(MUHelper::IsAttackSkillReady(ExtraOne));
    CharacterAttribute->Skill[1] = AT_SKILL_UNDEFINED;
    CHECK_FALSE(MUHelper::IsAttackSkillReady(ExtraOne));
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraTwo);
    CHECK_FALSE(MUHelper::IsAttackSkillReady(AT_SKILL_UNDEFINED));
    CHECK_FALSE(MUHelper::IsAttackSkillReady(static_cast<ActionSkillType>(MAX_SKILLS)));
}

TEST_CASE("Automatic eligibility retains attributes and skill-use conditions [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Strength = 501;
    gSkillManager.InvalidateSkillAttributeRequirementsCache();
    CHECK_FALSE(MUHelper::IsAttackSkillReady(ExtraOne));
    SkillAttribute[ExtraOne].Strength = 500;
    gSkillManager.InvalidateSkillAttributeRequirementsCache();
    CHECK(MUHelper::IsAttackSkillReady(ExtraOne));
    SkillAttribute[ExtraOne].SkillUseType = SKILL_USE_TYPE_BRAND;
    CHECK_FALSE(MUHelper::IsAttackSkillReady(ExtraOne));
}

TEST_CASE("Busy and approaching actions keep their slot while unsuccessful attempts advance [helper_rotation]")
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
    fixture.rotation.Complete(SkillResult::NotReady);
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
