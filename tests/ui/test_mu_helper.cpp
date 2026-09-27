#include "App/stdafx.h"

#include <doctest.h>

#include "Core/Globals/_enum.h"
#include "UI/MuHelper/MuHelperShared.h"

using UI::MuHelper::ClassFeatures;
using UI::MuHelper::ResolveClassFeatures;

// Each expectation below is one of native's RegisterBtnCharacter/RegisterBoxCharacter/
// RegisterIconCharacter/RegisterTextCharacter calls, restated per class.

TEST_CASE("every class but the dark lord has a third attack skill [ui][muhelper]")
{
    for (int c : { CLASS_WIZARD, CLASS_KNIGHT, CLASS_ELF, CLASS_DARK, CLASS_SUMMONER, CLASS_RAGEFIGHTER })
        CHECK(ResolveClassFeatures(c).skill3);
    CHECK_FALSE(ResolveClassFeatures(CLASS_DARK_LORD).skill3);
}

TEST_CASE("class-only controls belong to exactly their class [ui][muhelper]")
{
    const ClassFeatures dk = ResolveClassFeatures(CLASS_KNIGHT);
    CHECK(dk.combo);
    CHECK_FALSE(dk.pet);
    CHECK_FALSE(dk.party);

    const ClassFeatures dl = ResolveClassFeatures(CLASS_DARK_LORD);
    CHECK(dl.pet);
    CHECK_FALSE(dl.combo);
    CHECK_FALSE(dl.party);

    CHECK(ResolveClassFeatures(CLASS_ELF).autoHeal);
    CHECK(ResolveClassFeatures(CLASS_SUMMONER).drainLife);
    for (int c : { CLASS_WIZARD, CLASS_KNIGHT, CLASS_DARK, CLASS_DARK_LORD, CLASS_RAGEFIGHTER })
    {
        CHECK_FALSE(ResolveClassFeatures(c).autoHeal);
        CHECK_FALSE(ResolveClassFeatures(c).drainLife);
    }
}

TEST_CASE("party support is the wizard's and the elf's, each on its own page [ui][muhelper]")
{
    CHECK(ResolveClassFeatures(CLASS_WIZARD).party);
    CHECK(ResolveClassFeatures(CLASS_WIZARD).partyPage == SUB_PAGE_PARTY_CONFIG);
    CHECK(ResolveClassFeatures(CLASS_ELF).party);
    CHECK(ResolveClassFeatures(CLASS_ELF).partyPage == SUB_PAGE_PARTY_CONFIG_ELF);
    for (int c : { CLASS_KNIGHT, CLASS_DARK, CLASS_DARK_LORD, CLASS_SUMMONER, CLASS_RAGEFIGHTER })
        CHECK_FALSE(ResolveClassFeatures(c).party);
}

TEST_CASE("every class has one potion setting, opening its own page [ui][muhelper]")
{
    for (int c : { CLASS_WIZARD, CLASS_KNIGHT, CLASS_DARK, CLASS_DARK_LORD, CLASS_RAGEFIGHTER })
    {
        CHECK(ResolveClassFeatures(c).potionPage == SUB_PAGE_POTION_CONFIG);
        CHECK_FALSE(ResolveClassFeatures(c).potionSummoner);
    }
    CHECK(ResolveClassFeatures(CLASS_ELF).potionPage == SUB_PAGE_POTION_CONFIG_ELF);
    CHECK(ResolveClassFeatures(CLASS_SUMMONER).potionPage == SUB_PAGE_POTION_CONFIG_SUMMY);
    CHECK(ResolveClassFeatures(CLASS_SUMMONER).potionSummoner);
}

TEST_CASE("an empty condition group is filled, a chosen one is kept [ui][muhelper]")
{
    uint32_t bits = MUHelper::ON_CONDITION;
    UI::MuHelper::EnsureConditionDefaults(bits);
    CHECK((bits & ~MUHelper::MUHELPER_SKILL_PRECON_CLEAR) == MUHelper::ON_MOBS_NEARBY);
    CHECK((bits & ~MUHelper::MUHELPER_SKILL_SUBCON_CLEAR) == MUHelper::ON_MORE_THAN_TWO_MOBS);
    CHECK((bits & MUHelper::ON_CONDITION) != 0);

    uint32_t chosen = static_cast<uint32_t>(MUHelper::ON_CONDITION)
                    | static_cast<uint32_t>(MUHelper::ON_MOBS_ATTACKING)
                    | static_cast<uint32_t>(MUHelper::ON_MORE_THAN_FIVE_MOBS);
    const uint32_t before = chosen;
    UI::MuHelper::EnsureConditionDefaults(chosen);
    CHECK(chosen == before);
}

TEST_CASE("numeric fields parse whole numbers only, anything else as zero [ui][muhelper]")
{
    CHECK(UI::MuHelper::ParseIntInput(L"42") == 42);
    CHECK(UI::MuHelper::ParseIntInput(L"") == 0);
    CHECK(UI::MuHelper::ParseIntInput(L"4a") == 0);
    CHECK(UI::MuHelper::ParseIntInput(nullptr) == 0);
}
