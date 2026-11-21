#pragma once

#include "Core/Globals/_define.h"
#include "Data/GameData/MapData/Warp.h"

#include <array>
#include <list>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "Network/MoveCommandData.h"

struct tagITEM;

// The move rules of the warps (Data/Maps/Warps.json): what a character needs
// to warp, for the M window, the chat's /move and warp word and the control
// socket's warp. See docs/warp-list.md.
namespace GameLogic::Warps
{
// An equipped item as the rules see it; an empty slot has type -1.
struct EquippedItem
{
    int type = -1;
    int durability = 0;

    bool operator==(const EquippedItem&) const = default;
};

// The equipment slots; a fixed array, as the M window builds the hero's every frame.
using Equipment = std::array<EquippedItem, MAX_EQUIPMENT>;

// What the rules ask of the character.
struct WarpCharacter
{
    // Of the first stage (CLASS_WIZARD ... CLASS_RAGEFIGHTER).
    int baseClass = 0;
    int level = 0;
    int masterLevel = 0;
    int resets = 0;
    int zen = 0;
    int pkLevel = 0;
    bool gensMember = false;
    Equipment equipment{};

    bool operator==(const WarpCharacter&) const = default;
};

// The data entry of the warp `index`, which holds its rules: nullptr for a
// warp only the server has, which has none.
const Data::Maps::Warp* FindWarpRules(const Data::Maps::WarpList& list, int index);

// The level the character needs for a warp of `level`: lower for the classes
// of the class level discount, unless the warp has none.
int GetRequiredLevel(const Data::Maps::WarpList& list, const Data::Maps::Warp* rules, int level, int baseClass);

// The warp is only for Gens members here (the M window marks it).
bool IsGensWarp(const Data::Maps::Warp* rules, bool nonPvPServer);

// The equipment rules allow the warp: one of "needsOneOf" (not broken) is
// equipped, none of "notWith".
bool IsEquipmentAllowed(const Data::Maps::Warp& rules, std::span<const EquippedItem> equipment);

// Everything the M window checks for a warp of the list: its level, zen,
// master level and resets as the list has them (the server's with its list),
// the other rules from `list`: true when the character may warp there.
bool IsWarpAllowed(const Data::Maps::WarpList& list, const SEASON3B::CMoveCommandData::MOVEREQINFO& warp,
                   const WarpCharacter& character, bool nonPvPServer);

// The master level and resets the warp of the list needs ("ML n R n" in
// English), empty when it needs neither: the M window shows them under the
// level, on a second line of the row, as they don't fit beside it.
std::wstring GetMasterLevelAndResetsText(const SEASON3B::CMoveCommandData::MOVEREQINFO& warp);

// The chat's check of a text with /move or the warp word: the key of the
// message to show (see Data/GameData/MapData/WarpMessages.h; empty for the
// general one) when the equipment rules of the warp the text names block it.
// The warp is the first of the list whose shown name or /move name is the word
// after /move or the warp word, in any case (as /warp and the M window); when
// none is, the first whose shown name is in the text, or whose /move name is in
// it in any case. An empty name is in no text.
std::optional<std::string> CheckChatWarp(const wchar_t* chatText,
                                         const std::list<SEASON3B::CMoveCommandData::MOVEINFODATA*>& warps,
                                         const wchar_t* warpWord, const Data::Maps::WarpList& list,
                                         std::span<const EquippedItem> equipment);

// The equipment as the rules see it: the first `count` slots (at most MAX_EQUIPMENT), the others empty.
Equipment MakeEquipment(const tagITEM* equipment, int count);

// The character as the rules see it, from the game's values.
WarpCharacter MakeWarpCharacter(int playerClass, int level, unsigned long zen, int pkLevel, int gensInfluence,
                                int masterLevel, int resets, const tagITEM* equipment, int count);

// The hero, from the game's globals.
WarpCharacter MakeHeroWarpCharacter();
} // namespace GameLogic::Warps
