// Tests of how a held right button aims a direction skill at a ground point beyond its range.

#include "stdafx.h"

#include "doctest.h"

#include "GameLogic/Combat/CombatTarget.h"
#include "GameLogic/Combat/SkillAim.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"

#include <cmath>
#include <memory>

extern int SelectedCharacter, TargetX, TargetY;

using GameLogic::Combat::AimAtGround;
using GameLogic::Combat::CheckTarget;
using GameLogic::Combat::SkillAim::AutoAttackState;
using GameLogic::Combat::SkillAim::IsAimedByDirection;
using GameLogic::Combat::SkillAim::PullIntoRange;
using GameLogic::Combat::SkillAim::RepeatsAutoAttack;

namespace
{
constexpr float Tile = TERRAIN_SCALE;

// The real CheckTile on the tile the point falls on, as the attack code tests it.
bool TileCentreInRange(const float origin[2], const float point[2], float rangeTiles)
{
    CHARACTER caster{};
    caster.Object.Position[0] = origin[0];
    caster.Object.Position[1] = origin[1];
    const int previousTargetX = TargetX;
    const int previousTargetY = TargetY;
    TargetX = static_cast<int>(point[0] / Tile);
    TargetY = static_cast<int>(point[1] / Tile);
    const bool inRange = CheckTile(&caster, &caster.Object, rangeTiles);
    TargetX = previousTargetX;
    TargetY = previousTargetY;
    return inRange;
}

// The hero and a character under the cursor, restoring the globals afterwards.
struct SelectionFixture
{
    static constexpr int HeroIndex = 0;
    static constexpr int SelectedIndex = 1;
    static constexpr float HeroTile = 120.0f;

    std::unique_ptr<CHARACTER[]> characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    CHARACTER* previousCharacters = CharactersClient;
    CHARACTER* previousHero = Hero;
    int previousSelected = SelectedCharacter;
    int previousTargetX = TargetX;
    int previousTargetY = TargetY;

    SelectionFixture()
    {
        CharactersClient = characters.get();
        Hero = &characters[HeroIndex];
        Hero->Object.Position[0] = Hero->Object.Position[1] = (HeroTile + 0.5f) * Tile;
        SelectedCharacter = SelectedIndex;
    }

    ~SelectionFixture()
    {
        CharactersClient = previousCharacters;
        Hero = previousHero;
        SelectedCharacter = previousSelected;
        TargetX = previousTargetX;
        TargetY = previousTargetY;
    }

    // Places a monster, or an NPC that can't be attacked, `tiles` east of the hero.
    void Select(float tiles, bool attackable)
    {
        CHARACTER& selected = characters[SelectedIndex];
        selected.Object.Kind = attackable ? KIND_MONSTER : KIND_NPC;
        selected.Object.Position[0] = Hero->Object.Position[0] + tiles * Tile;
        selected.Object.Position[1] = Hero->Object.Position[1];
    }
};
} // namespace

TEST_CASE("The skills held on the ground toward the cursor are direction skills [gamelogic][skills]")
{
    CHECK(IsAimedByDirection(AT_SKILL_EVIL_SPIRIT));
    CHECK(IsAimedByDirection(AT_SKILL_EVIL_SPIRIT_STR_MG));
    CHECK(IsAimedByDirection(AT_SKILL_TRIPLE_SHOT));
    CHECK(IsAimedByDirection(AT_SKILL_TRIPLE_SHOT_MASTERY));
    CHECK(IsAimedByDirection(AT_SKILL_POWER_SLASH));
    CHECK(IsAimedByDirection(AT_SKILL_POWER_SLASH_STR));
    CHECK(IsAimedByDirection(AT_SKILL_FIRE_SLASH));
    CHECK(IsAimedByDirection(AT_SKILL_THUNDER_STRIKE));
    CHECK(IsAimedByDirection(AT_SKILL_FLAME_STRIKE));
    CHECK(IsAimedByDirection(AT_SKILL_GIGANTIC_STORM));
    CHECK(IsAimedByDirection(AT_SKILL_LIGHTNING_SHOCK));
    CHECK(IsAimedByDirection(AT_SKILL_LIGHTNING_SHOCK_STR));

    // Skills cast on a chosen spot or creature keep their range.
    CHECK_FALSE(IsAimedByDirection(AT_SKILL_DECAY));
    CHECK_FALSE(IsAimedByDirection(AT_SKILL_ICE_STORM));
    CHECK_FALSE(IsAimedByDirection(AT_SKILL_NOVA));
    CHECK_FALSE(IsAimedByDirection(AT_SKILL_HEALING));
}

TEST_CASE("A ground point beyond the range is pulled back along its direction [gamelogic][skills]")
{
    const float origin[2] = {12050.0f, 8050.0f};
    for (const float angle : {0.0f, 0.7f, 1.9f, 3.3f, 4.4f, 5.9f})
    {
        float point[2] = {origin[0] + std::cos(angle) * 1500.0f, origin[1] + std::sin(angle) * 1500.0f};
        CHECK(PullIntoRange(origin, point, 6.0f, Tile));

        const float dx = point[0] - origin[0];
        const float dy = point[1] - origin[1];
        CHECK(std::sqrt(dx * dx + dy * dy) == doctest::Approx(500.0f));
        CHECK(std::atan2(dy, dx) == doctest::Approx(std::atan2(std::sin(angle), std::cos(angle))));
        CHECK(TileCentreInRange(origin, point, 6.0f));
    }
}

TEST_CASE("A ground point within the range is left where it is [gamelogic][skills]")
{
    const float origin[2] = {5000.0f, 5000.0f};
    float point[2] = {5300.0f, 5100.0f};
    CHECK_FALSE(PullIntoRange(origin, point, 6.0f, Tile));
    CHECK(point[0] == 5300.0f);
    CHECK(point[1] == 5100.0f);

    float tooShortRange[2] = {9000.0f, 9000.0f};
    CHECK_FALSE(PullIntoRange(origin, tooShortRange, 1.0f, Tile));
}

TEST_CASE("A direction skill held on the ground beyond its range is aimed within it [gamelogic][skills]")
{
    SelectionFixture fixture;
    constexpr float Range = 6.0f;
    const float ground[3] = {Hero->Object.Position[0] - 4.0f * Tile, Hero->Object.Position[1] + 11.0f * Tile, 0.0f};

    AimAtGround(Hero, AT_SKILL_EVIL_SPIRIT, Range, ground);
    CHECK(CheckTile(Hero, &Hero->Object, Range));
    CHECK(Hero->TargetPosition[0] < Hero->Object.Position[0]);
    CHECK(Hero->TargetPosition[1] > Hero->Object.Position[1]);

    // A skill cast on a chosen spot keeps the far point, so the hero walks there first.
    AimAtGround(Hero, AT_SKILL_DECAY, Range, ground);
    CHECK_FALSE(CheckTile(Hero, &Hero->Object, Range));
    CHECK(Hero->TargetPosition[0] == ground[0]);
    CHECK(Hero->TargetPosition[1] == ground[1]);
}

TEST_CASE("A direction skill aimed at a character that can't be attacked beyond its range is pulled into range "
          "[gamelogic][skills]")
{
    SelectionFixture fixture;
    constexpr float Range = 6.0f;
    fixture.Select(9.0f, false);

    REQUIRE(CheckTarget(Hero, AT_SKILL_EVIL_SPIRIT, Range));

    CHECK(CheckTile(Hero, &Hero->Object, Range));
    CHECK(Hero->TargetPosition[0] > Hero->Object.Position[0]);
    CHECK(Hero->TargetPosition[1] == doctest::Approx(Hero->Object.Position[1]));
}

TEST_CASE("The range a caller passes, such as the dark horse's extra two tiles, sets how far the aim is pulled "
          "[gamelogic][skills]")
{
    SelectionFixture fixture;
    constexpr float RangeOnDarkHorse = 6.0f + 2.0f;
    fixture.Select(12.0f, false);

    REQUIRE(CheckTarget(Hero, AT_SKILL_EARTHSHAKE, RangeOnDarkHorse));

    CHECK(CheckTile(Hero, &Hero->Object, RangeOnDarkHorse));
    CHECK_FALSE(CheckTile(Hero, &Hero->Object, 6.0f));
}

TEST_CASE("Other skills and attackable characters keep the selected character as the aim [gamelogic][skills]")
{
    SelectionFixture fixture;
    constexpr float Range = 6.0f;
    fixture.Select(9.0f, false);
    REQUIRE(CheckTarget(Hero, AT_SKILL_DECAY, Range));
    CHECK_FALSE(CheckTile(Hero, &Hero->Object, Range));

    fixture.Select(9.0f, true);
    REQUIRE(CheckTarget(Hero, AT_SKILL_EVIL_SPIRIT, Range));
    CHECK_FALSE(CheckTile(Hero, &Hero->Object, Range));
}

TEST_CASE("Auto-attack repeats a direction skill only while the character under the cursor can be attacked "
          "[gamelogic][skills]")
{
    AutoAttackState state;
    state.skill = AT_SKILL_EVIL_SPIRIT;
    state.optionOn = true;
    state.mapAllowsIt = true;
    state.afterSkillCast = true;
    state.hasSelection = true;
    state.selectionAttackable = true;
    CHECK(RepeatsAutoAttack(state));

    SUBCASE("a party member or NPC under the cursor stops it")
    {
        state.selectionAttackable = false;
        CHECK_FALSE(RepeatsAutoAttack(state));
    }
    SUBCASE("the option is off")
    {
        state.optionOn = false;
        CHECK_FALSE(RepeatsAutoAttack(state));
    }
    SUBCASE("the stadium or Chaos Castle")
    {
        state.mapAllowsIt = false;
        CHECK_FALSE(RepeatsAutoAttack(state));
    }
    SUBCASE("no skill was cast")
    {
        state.afterSkillCast = false;
        CHECK_FALSE(RepeatsAutoAttack(state));
    }
    SUBCASE("nothing under the cursor")
    {
        state.hasSelection = false;
        state.selectionAttackable = false;
        CHECK_FALSE(RepeatsAutoAttack(state));
    }
}

TEST_CASE("Auto-attack keeps repeating other skills on a character that can't be attacked [gamelogic][skills]")
{
    AutoAttackState state;
    state.optionOn = true;
    state.mapAllowsIt = true;
    state.afterSkillCast = true;
    state.hasSelection = true;
    state.selectionAttackable = false;

    state.skill = AT_SKILL_SOUL_BARRIER;
    CHECK(RepeatsAutoAttack(state));

    state.skill = AT_SKILL_POWER_SLASH;
    CHECK_FALSE(RepeatsAutoAttack(state));
}
