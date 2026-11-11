#include <doctest.h>

#include "Core/Platform/WinCompat.h" // BYTE, used by _enum.h
#include "Core/Globals/_enum.h"
#include "UI/HUD/Skills/SkillIconAtlas.h"

using namespace UI::Skills::Icon;

namespace
{
SkillIconSource Plain(int skillType)
{
    return {.skillType = skillType, .skillUseType = SKILL_USE_TYPE_NONE, .magicIcon = 0, .usable = true};
}

SkillIconSource Grey(int skillType)
{
    return {.skillType = skillType, .skillUseType = SKILL_USE_TYPE_NONE, .magicIcon = 0, .usable = false};
}

void CheckCell(const SkillIcon& icon, Atlas atlas, int column, int row)
{
    CHECK(icon.atlas == atlas);
    CHECK(icon.column == column);
    CHECK(icon.row == row);
}
} // namespace

TEST_CASE("skill 0 has no icon [ui][skills]")
{
    const SkillIcon icon = ResolveSkillIcon(Plain(0));
    CHECK(icon.atlas == Atlas::None);
    CHECK(IconSpriteName(icon).empty());
}

TEST_CASE("early skills count through the first atlas eight per row [ui][skills]")
{
    // Falling Slash 19 -> cell 18, Death Stab 43 -> cell 42.
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_FALLING_SLASH)), Atlas::Skill1, 2, 2);
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_DEATHSTAB)), Atlas::Skill1, 2, 5);
    CHECK(IconSpriteName(ResolveSkillIcon(Plain(AT_SKILL_FALLING_SLASH))) == "skill-icon-skill1-lit-2-2");
    CHECK(IconSpriteName(ResolveSkillIcon(Grey(AT_SKILL_FALLING_SLASH))) == "skill-icon-skill1-grey-2-2");
}

TEST_CASE("master strengtheners draw their Magic_Icon cell from the master atlas [ui][skills]")
{
    const SkillIcon cyclone = ResolveSkillIcon(
        {.skillType = AT_SKILL_CYCLONE_STR, .skillUseType = SKILL_USE_TYPE_MASTER, .magicIcon = 137, .usable = true});
    CheckCell(cyclone, Atlas::Master, 12, 5);
    CHECK(IconSpriteName(cyclone) == "master-icon-lit-137");

    const SkillIcon twisting = ResolveSkillIcon({.skillType = AT_SKILL_TWISTING_SLASH_STR,
                                                 .skillUseType = SKILL_USE_TYPE_MASTER,
                                                 .magicIcon = 26,
                                                 .usable = false});
    CHECK(IconSpriteName(twisting) == "master-icon-grey-26");
}

TEST_CASE("pet commands and the Fenrir storm use the command atlas [ui][skills]")
{
    CheckCell(ResolveSkillIcon(Plain(AT_PET_COMMAND_DEFAULT)), Atlas::Command, 0, 0);
    CheckCell(ResolveSkillIcon(Plain(AT_PET_COMMAND_DEFAULT + 3)), Atlas::Command, 3, 0);
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_PLASMA_STORM_FENRIR)), Atlas::Command, 4, 0);
    CHECK(IconSpriteName(ResolveSkillIcon(Grey(AT_PET_COMMAND_DEFAULT + 1))) == "skill-icon-command-grey-1-0");
}

TEST_CASE("summoner skills sit on the second atlas's fourth row [ui][skills]")
{
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_ALICE_DRAINLIFE)), Atlas::Skill2, 0, 3);
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_ALICE_SLEEP)), Atlas::Skill2, 4, 3);
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_ALICE_BERSERKER)), Atlas::Skill2, 10, 3);
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_ALICE_ENERVATION)), Atlas::Skill2, 9, 3);
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_SUMMON_EXPLOSION)), Atlas::Skill2, 6, 3);
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_SUMMON_POLLUTION)), Atlas::Skill2, 11, 3);
}

TEST_CASE("Strike of Destruction has its own cell, not its number's [ui][skills]")
{
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_STRIKE_OF_DESTRUCTION)), Atlas::Skill2, 7, 2);
}

TEST_CASE("master active skills count Magic_Icon twelve per row from row four [ui][skills]")
{
    CheckCell(ResolveSkillIcon({.skillType = AT_SKILL_SPIRAL_SLASH,
                                .skillUseType = SKILL_USE_TYPE_MASTERACTIVE,
                                .magicIcon = 13,
                                .usable = true}),
              Atlas::Skill2, 1, 5);
}

TEST_CASE("rage fighter skills use the third atlas twelve per row [ui][skills]")
{
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_KILLING_BLOW)), Atlas::Skill3, 0, 0);
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_KILLING_BLOW + 13)), Atlas::Skill3, 1, 1);
}

TEST_CASE("skills from Spiral Slash on use the second atlas eight per row [ui][skills]")
{
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_SPIRAL_SLASH)), Atlas::Skill2, 0, 0);
    CheckCell(ResolveSkillIcon(Plain(AT_SKILL_SPIRAL_SLASH + 10)), Atlas::Skill2, 2, 1);
}
