#include "App/stdafx.h"

#include <doctest.h>

#include "Core/Globals/_enum.h"
#include "UI/HUD/Skills/SkillIconCell.h"

using UI::Skills::IconCell;
using UI::Skills::IconSheet;
using UI::Skills::ResolveIconCell;

// Every expected cell below is native's own UV formula divided back into a grid position:
// fU = col * 20 / 256, fV = row * 28 / 256 (CSkillList::RenderSkillIcon()).

TEST_CASE("an empty slot resolves to no icon [ui][skills]")
{
    CHECK(ResolveIconCell(0, 0, 0) == IconCell{});
    CHECK(ResolveIconCell(-1, 0, 0) == IconCell{});
}

TEST_CASE("master-range skills take the master sheet whatever the cascade would say [ui][skills]")
{
    // Native tested this after the cascade and let it override. These three are the skills the old
    // MU Helper copies disagreed on -- all past AT_SKILL_MASTER_BEGIN, so the disagreement was dead.
    CHECK(ResolveIconCell(AT_SKILL_ALICE_SLEEP_STR, 0, 53) == IconCell{ IconSheet::Master, 3, 2 });
    CHECK(ResolveIconCell(AT_SKILL_LIGHTNING_SHOCK_STR, 0, 53) == IconCell{ IconSheet::Master, 3, 2 });
    CHECK(ResolveIconCell(AT_SKILL_ALICE_BERSERKER_STR, 0, 53) == IconCell{ IconSheet::Master, 3, 2 });

    // 25 cells per row on the 512 sheet.
    CHECK(ResolveIconCell(AT_SKILL_MASTER_BEGIN, 0, 24) == IconCell{ IconSheet::Master, 24, 0 });
    CHECK(ResolveIconCell(AT_SKILL_MASTER_BEGIN, 0, 25) == IconCell{ IconSheet::Master, 0, 1 });

    // skillUseType 4 would otherwise send it to the Skill2 branch; the master test comes first.
    CHECK(ResolveIconCell(AT_SKILL_MASTER_BEGIN, 4, 7) == IconCell{ IconSheet::Master, 7, 0 });
}

TEST_CASE("pet commands and the Fenrir storm use the command sheet [ui][skills]")
{
    CHECK(ResolveIconCell(AT_PET_COMMAND_DEFAULT, 0, 0) == IconCell{ IconSheet::Command, 0, 0 });
    CHECK(ResolveIconCell(AT_PET_COMMAND_DEFAULT + 2, 0, 0) == IconCell{ IconSheet::Command, 2, 0 });
    CHECK(ResolveIconCell(AT_SKILL_PLASMA_STORM_FENRIR, 0, 0) == IconCell{ IconSheet::Command, 4, 0 });
}

TEST_CASE("the summoner and fixed-position skills sit where native put them [ui][skills]")
{
    CHECK(ResolveIconCell(AT_SKILL_ALICE_DRAINLIFE, 0, 0) == IconCell{ IconSheet::Skill2, 0, 3 });
    CHECK(ResolveIconCell(AT_SKILL_ALICE_SLEEP, 0, 0) == IconCell{ IconSheet::Skill2, 4, 3 });
    CHECK(ResolveIconCell(AT_SKILL_ALICE_BERSERKER, 0, 0) == IconCell{ IconSheet::Skill2, 10, 3 });
    // The one range native did NOT wrap with % 8.
    CHECK(ResolveIconCell(AT_SKILL_ALICE_WEAKNESS, 0, 0) == IconCell{ IconSheet::Skill2, 8, 3 });
    CHECK(ResolveIconCell(AT_SKILL_SUMMON_POLLUTION, 0, 0) == IconCell{ IconSheet::Skill2, 11, 3 });
    CHECK(ResolveIconCell(AT_SKILL_STRIKE_OF_DESTRUCTION, 0, 0) == IconCell{ IconSheet::Skill2, 7, 2 });
    CHECK(ResolveIconCell(AT_SKILL_CHAOTIC_DISEIER, 0, 0) == IconCell{ IconSheet::Skill2, 3, 8 });
    CHECK(ResolveIconCell(AT_SKILL_RECOVER, 0, 0) == IconCell{ IconSheet::Skill2, 9, 2 });
    CHECK(ResolveIconCell(AT_SKILL_MULTI_SHOT, 0, 0) == IconCell{ IconSheet::Skill2, 0, 8 });
    CHECK(ResolveIconCell(AT_SKILL_FLAME_STRIKE, 0, 0) == IconCell{ IconSheet::Skill2, 1, 8 });
    CHECK(ResolveIconCell(AT_SKILL_GIGANTIC_STORM, 0, 0) == IconCell{ IconSheet::Skill2, 2, 8 });
    CHECK(ResolveIconCell(AT_SKILL_LIGHTNING_SHOCK, 0, 0) == IconCell{ IconSheet::Skill2, 2, 3 });
    CHECK(ResolveIconCell(AT_SKILL_EXPANSION_OF_WIZARDRY, 0, 0) == IconCell{ IconSheet::Skill2, 8, 2 });
}

TEST_CASE("skill use type 4 is addressed by its icon, 12 per row from row 4 [ui][skills]")
{
    CHECK(ResolveIconCell(5, 4, 0) == IconCell{ IconSheet::Skill2, 0, 4 });
    CHECK(ResolveIconCell(5, 4, 13) == IconCell{ IconSheet::Skill2, 1, 5 });
}

TEST_CASE("the three id ranges fall through in native's order [ui][skills]")
{
    CHECK(ResolveIconCell(AT_SKILL_KILLING_BLOW, 0, 0) == IconCell{ IconSheet::Skill3, 0, 0 });
    CHECK(ResolveIconCell(AT_SKILL_SPIRAL_SLASH, 0, 0) == IconCell{ IconSheet::Skill2, 0, 0 });
    CHECK(ResolveIconCell(1, 0, 0) == IconCell{ IconSheet::Skill1, 0, 0 });
    CHECK(ResolveIconCell(9, 0, 0) == IconCell{ IconSheet::Skill1, 0, 1 });
}

TEST_CASE("sprite names match what the sheet generator emits [ui][skills]")
{
    CHECK(UI::Skills::IconSpriteName(IconCell{ IconSheet::Skill2, 3, 8 }) == "skill-icon-skill2-3-8");
    CHECK(UI::Skills::IconSpriteName(IconCell{ IconSheet::Master, 24, 17 }) == "skill-icon-master-24-17");
    CHECK(UI::Skills::IconSpriteName(IconCell{ IconSheet::Command, 0, 0 }) == "skill-icon-command-0-0");
    CHECK(UI::Skills::IconSpriteName(IconCell{}).empty());
}
