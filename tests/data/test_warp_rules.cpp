#include "stdafx.h"

#include "doctest.h"
#include "ShippedItemsScope.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/MapData/WarpMessages.h"
#include "Data/GameData/MapData/WarpsJson.h"
#include "GameLogic/Warps/WarpRules.h"
#include "I18N/All.h"

#include <filesystem>
#include <list>
#include <string>
#include <vector>

using Data::Maps::Warp;
using Data::Maps::WarpItemMatch;
using Data::Maps::WarpList;
using GameLogic::Warps::EquippedItem;
using GameLogic::Warps::IsWarpAllowed;
using GameLogic::Warps::WarpCharacter;

namespace
{
// The warp with the tag values its names have, as the reader and the game set them.
Warp Resolved(const Warp& warp)
{
    WarpList list;
    list.warps.push_back(warp);
    Data::Maps::ResolveItemTags(list);
    return list.warps[0];
}

WarpList OneWarp(const Warp& warp)
{
    WarpList list;
    list.classLevelDiscounts.push_back({CLASS_DARK_LORD, 2, 3});
    list.warps.push_back(warp);
    return list;
}

Warp MakeWarp(int index, int level)
{
    Warp warp;
    warp.index = index;
    warp.names.Set("en", "Test");
    warp.command = "Test";
    warp.level = level;
    warp.zen = 100;
    return warp;
}

WarpCharacter Strong()
{
    WarpCharacter character;
    character.level = 400;
    character.zen = 100000;
    return character;
}

bool CanMove(const WarpList& list, const WarpCharacter& character, bool nonPvP = false)
{
    return IsWarpAllowed(list, SEASON3B::CMoveCommandData::MakeMoveReqInfo(list.warps[0], "en"), character, nonPvP);
}

} // namespace

TEST_CASE("Warp rules: a master level and resets close the warp below them [warps][rules]")
{
    Warp warp = MakeWarp(60, 10);
    warp.minMasterLevel = 50;
    warp.minResets = 3;
    const WarpList list = OneWarp(warp);

    WarpCharacter character = Strong();
    character.masterLevel = 50;
    character.resets = 3;
    CHECK(CanMove(list, character));
    character.masterLevel = 49;
    CHECK_FALSE(CanMove(list, character));
    character.masterLevel = 50;
    character.resets = 2;
    CHECK_FALSE(CanMove(list, character));

    // 0 is none.
    const WarpList open = OneWarp(MakeWarp(60, 10));
    CHECK(CanMove(open, Strong()));
}

// With a server's list the list's values count (the server's), not the data file's.
TEST_CASE("Warp rules: the master level and resets of the list count, not the data file's [warps][rules]")
{
    Warp warp = MakeWarp(60, 10);
    warp.minMasterLevel = 50;
    const WarpList list = OneWarp(warp);
    auto listed = SEASON3B::CMoveCommandData::MakeMoveReqInfo(warp, "en");
    listed.iReqMasterLevel = 0;
    listed.iReqResets = 4;

    WarpCharacter character = Strong();
    character.resets = 3;
    CHECK_FALSE(IsWarpAllowed(list, listed, character, false));
    character.resets = 4;
    CHECK(IsWarpAllowed(list, listed, character, false));

    // A warp only the server has has no rules, but its master level and resets count.
    auto serverOnly = listed;
    serverOnly.index = 300;
    serverOnly.iReqMasterLevel = 20;
    CHECK_FALSE(IsWarpAllowed(list, serverOnly, character, false));
    character.masterLevel = 20;
    CHECK(IsWarpAllowed(list, serverOnly, character, false));
    CHECK(GameLogic::Warps::GetMasterLevelAndResetsText(serverOnly) == L"ML 20 R 4");
}

TEST_CASE("Warp rules: the level column's second line has the master level and resets only when set [warps][rules]")
{
    I18N::SetLocale("en");
    Warp warp = MakeWarp(60, 10);
    const auto text = [&] { return GameLogic::Warps::GetMasterLevelAndResetsText(
                                SEASON3B::CMoveCommandData::MakeMoveReqInfo(warp, "en")); };
    CHECK(text().empty());
    warp.minMasterLevel = 50;
    CHECK(text() == L"ML 50");
    warp.minResets = 3;
    CHECK(text() == L"ML 50 R 3");
    warp.minMasterLevel = 0;
    CHECK(text() == L"R 3");
    // In the UI language.
    I18N::SetLocale("es");
    warp.minMasterLevel = 50;
    CHECK(text() == L"NM 50 R 3");
    I18N::SetLocale("en");
}

TEST_CASE("Warp rules: the class discount follows the data, unless the warp has none [warps][rules]")
{
    Warp warp = MakeWarp(60, 100);
    WarpList list = OneWarp(warp);
    CHECK(GameLogic::Warps::GetRequiredLevel(list, &list.warps[0], 100, CLASS_DARK_LORD) == 66);
    CHECK(GameLogic::Warps::GetRequiredLevel(list, &list.warps[0], 100, CLASS_WIZARD) == 100);
    CHECK(GameLogic::Warps::GetRequiredLevel(list, nullptr, 100, CLASS_DARK_LORD) == 66);
    list.warps[0].noClassDiscount = true;
    CHECK(GameLogic::Warps::GetRequiredLevel(list, &list.warps[0], 100, CLASS_DARK_LORD) == 100);
    list.classLevelDiscounts[0] = {CLASS_DARK_LORD, 1, 2};
    CHECK(GameLogic::Warps::GetRequiredLevel(list, nullptr, 99, CLASS_DARK_LORD) == 49);
    // 20001 * 999 doesn't fit a float: it rounds to 19981000 before the division.
    list.classLevelDiscounts[0] = {CLASS_DARK_LORD, 999, 1000};
    CHECK(GameLogic::Warps::GetRequiredLevel(list, nullptr, 20001, CLASS_DARK_LORD) == 19980);
}

TEST_CASE("Warp rules: PvP servers only and Gens only [warps][rules]")
{
    Warp warp = MakeWarp(60, 10);
    warp.pvpServersOnly = true;
    WarpCharacter character = Strong();
    CHECK(CanMove(OneWarp(warp), character, false));
    CHECK_FALSE(CanMove(OneWarp(warp), character, true));

    warp.pvpServersOnly = false;
    warp.gensOnly = true;
    CHECK_FALSE(CanMove(OneWarp(warp), character, false));
    CHECK(CanMove(OneWarp(warp), character, true));
    character.gensMember = true;
    CHECK(CanMove(OneWarp(warp), character, false));
    CHECK(GameLogic::Warps::IsGensWarp(&warp, false));
    CHECK_FALSE(GameLogic::Warps::IsGensWarp(&warp, true));
}

TEST_CASE("Warp rules: needs one of counts unbroken items, not with any, except leaves out a tag [warps][rules]")
{
    ShippedItemsScope items;
    Warp warp = MakeWarp(60, 10);
    warp.needsOneOf.push_back({WarpItemMatch::Kind::Wings});
    WarpItemMatch rings{WarpItemMatch::Kind::Tag};
    rings.tag = "transformationRing";
    rings.exceptTag = "pandaOrSkeleton";
    warp.notWith.push_back(rings);
    warp.notWith.push_back({WarpItemMatch::Kind::Item, 13, 2});
    warp = Resolved(warp);

    std::vector<EquippedItem> equipment(MAX_EQUIPMENT);
    CHECK_FALSE(GameLogic::Warps::IsEquipmentAllowed(warp, equipment));
    equipment[EQUIPMENT_WING] = {ITEM_WINGS_OF_ELF, 0};
    CHECK_FALSE(GameLogic::Warps::IsEquipmentAllowed(warp, equipment));
    equipment[EQUIPMENT_WING] = {ITEM_WINGS_OF_ELF, 1};
    CHECK(GameLogic::Warps::IsEquipmentAllowed(warp, equipment));
    equipment[EQUIPMENT_RING_LEFT] = {ITEM_PANDA_TRANSFORMATION_RING, 0};
    CHECK(GameLogic::Warps::IsEquipmentAllowed(warp, equipment));
    equipment[EQUIPMENT_RING_LEFT] = {ITEM_TRANSFORMATION_RING, 0};
    CHECK_FALSE(GameLogic::Warps::IsEquipmentAllowed(warp, equipment));
    equipment[EQUIPMENT_RING_LEFT] = {-1, 0};
    equipment[EQUIPMENT_HELPER] = {ITEM_HORN_OF_UNIRIA, 0};
    CHECK_FALSE(GameLogic::Warps::IsEquipmentAllowed(warp, equipment));

    // An unknown tag matches nothing.
    Warp unknown = MakeWarp(61, 10);
    WarpItemMatch nothing{WarpItemMatch::Kind::Tag};
    nothing.tag = "noSuchTag";
    unknown.needsOneOf.push_back(nothing);
    unknown = Resolved(unknown);
    CHECK_FALSE(GameLogic::Warps::IsEquipmentAllowed(unknown, equipment));
}

TEST_CASE("Warp rules: the blocked message is the warp's text, the general one when empty or unknown [warps][rules]")
{
    I18N::SetLocale("en");
    CHECK(std::wstring(Data::Maps::GetWarpMessageText("")) == L"You can't warp there with your equipment.");
    CHECK(std::wstring(Data::Maps::GetWarpMessageText("NoSuchText")) == L"You can't warp there with your equipment.");
    CHECK(Data::Maps::GetWarpMessageText("YouCannotGoToAtlansWhileRidingAUnicorn") ==
          I18N::Game::YouCannotGoToAtlansWhileRidingAUnicorn);
    CHECK(Data::Maps::IsWarpMessageKey("YouCanEnterIcarusOnlyWithWingsDinorantFenrirr"));
    CHECK_FALSE(Data::Maps::IsWarpMessageKey("Icarus"));
    I18N::SetLocale("de");
    CHECK(std::wstring(Data::Maps::GetWarpMessageText("")) ==
          L"Mit Ihrer Ausrüstung können Sie sich nicht dorthin teleportieren.");
    I18N::SetLocale("en");
}

// /move checks the equipment as the M window does, "except" too: wings and a panda or skeleton ring may go to Icarus.
TEST_CASE("Warp rules: the chat's /move leaves out the except tag's items as the M window does [warps][rules]")
{
    ShippedItemsScope items;
    Warp icarus = MakeWarp(23, 10);
    icarus.names.Set("en", "Icarus");
    icarus.command = "Icarus";
    icarus.needsOneOf.push_back({WarpItemMatch::Kind::Wings});
    WarpItemMatch rings{WarpItemMatch::Kind::Tag};
    rings.tag = "transformationRing";
    rings.exceptTag = "pandaOrSkeleton";
    icarus.notWith.push_back(rings);
    icarus.blockedMessage = "YouCanEnterIcarusOnlyWithWingsDinorantFenrirr";
    icarus = Resolved(icarus);
    const WarpList list = OneWarp(icarus);

    SEASON3B::CMoveCommandData::MOVEINFODATA entry{};
    entry._ReqInfo = SEASON3B::CMoveCommandData::MakeMoveReqInfo(icarus, "en");
    const std::list<SEASON3B::CMoveCommandData::MOVEINFODATA*> warps{&entry};
    const auto check = [&](const std::vector<EquippedItem>& equipment)
    { return GameLogic::Warps::CheckChatWarp(L"/move Icarus", warps, L"", list, equipment); };

    std::vector<EquippedItem> equipment(MAX_EQUIPMENT);
    equipment[EQUIPMENT_WING] = {ITEM_WINGS_OF_ELF, 1};
    CHECK_FALSE(check(equipment).has_value());
    equipment[EQUIPMENT_RING_LEFT] = {ITEM_PANDA_TRANSFORMATION_RING, 1};
    CHECK_FALSE(check(equipment).has_value());
    CHECK(GameLogic::Warps::IsEquipmentAllowed(icarus, equipment));
    equipment[EQUIPMENT_RING_LEFT] = {ITEM_TRANSFORMATION_RING, 1};
    CHECK(check(equipment) == icarus.blockedMessage);
    CHECK_FALSE(GameLogic::Warps::IsEquipmentAllowed(icarus, equipment));
}

// "/move Atlans2" checks Atlans2's rules, though "Atlans", earlier in the list, is in the text too.
TEST_CASE("Warp rules: the chat checks the warp the word after /move names before one whose name is in the text [warps][rules]")
{
    ShippedItemsScope items;
    Warp atlans = MakeWarp(11, 10);
    atlans.names.Set("en", "Atlans");
    atlans.command = "Atlans";
    Warp atlans2 = MakeWarp(12, 10);
    atlans2.names.Set("en", "Atlans2");
    atlans2.command = "Atlans2";
    atlans2.needsOneOf.push_back({WarpItemMatch::Kind::Wings});
    WarpList list;
    list.warps = {atlans, atlans2};

    SEASON3B::CMoveCommandData::MOVEINFODATA first{};
    first._ReqInfo = SEASON3B::CMoveCommandData::MakeMoveReqInfo(atlans, "en");
    SEASON3B::CMoveCommandData::MOVEINFODATA second{};
    second._ReqInfo = SEASON3B::CMoveCommandData::MakeMoveReqInfo(atlans2, "en");
    const std::list<SEASON3B::CMoveCommandData::MOVEINFODATA*> warps{&first, &second};
    const std::vector<EquippedItem> equipment(MAX_EQUIPMENT);
    const auto blocked = [&](const wchar_t* text)
    { return GameLogic::Warps::CheckChatWarp(text, warps, L"/WARP", list, equipment).has_value(); };

    CHECK(blocked(L"/move Atlans2"));
    CHECK(blocked(L"/move  atlans2"));
    CHECK(blocked(L"/WARP Atlans2"));
    // The shown name in any case too, as /warp takes it (with another /move name for Atlans2).
    Warp renamed = atlans2;
    renamed.command = "Other";
    second._ReqInfo = SEASON3B::CMoveCommandData::MakeMoveReqInfo(renamed, "en");
    CHECK(blocked(L"/move ATLANS2"));
    CHECK_FALSE(blocked(L"/move Atlans"));
    // No warp is the word: the first whose name is in the text, as the original client.
    CHECK_FALSE(blocked(L"/move Atlans2x"));
}

// While the editor's name or /move name field is empty, that warp's rules block no other warp.
TEST_CASE("Warp rules: a warp with an empty name or /move name is in no chat text [warps][rules]")
{
    ShippedItemsScope items;
    Warp blocking = MakeWarp(23, 10);
    blocking.needsOneOf.push_back({WarpItemMatch::Kind::Wings});
    const WarpList list = OneWarp(blocking);
    const std::vector<EquippedItem> equipment(MAX_EQUIPMENT);

    for (const bool emptyName : {true, false})
    {
        Warp shown = blocking;
        shown.names.Set("en", emptyName ? "" : "Icarus");
        shown.command = emptyName ? "Icarus" : "";
        SEASON3B::CMoveCommandData::MOVEINFODATA entry{};
        entry._ReqInfo = SEASON3B::CMoveCommandData::MakeMoveReqInfo(shown, "en");
        const std::list<SEASON3B::CMoveCommandData::MOVEINFODATA*> warps{&entry};
        CAPTURE(emptyName);
        CHECK_FALSE(GameLogic::Warps::CheckChatWarp(L"/move Lorencia", warps, L"", list, equipment).has_value());
        CHECK(GameLogic::Warps::CheckChatWarp(L"/move Icarus", warps, L"", list, equipment).has_value());
    }
}
