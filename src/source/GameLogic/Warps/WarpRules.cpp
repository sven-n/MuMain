#include "stdafx.h"

#include "GameLogic/Warps/WarpRules.h"

#include "Character/CharacterManager.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

#include <algorithm>
#include <cstdint>
#include <cwchar>
#include <cwctype>
#include <iterator>
#include <string_view>

namespace GameLogic::Warps
{
namespace
{
using Data::Items::ItemTag;
using Data::Maps::Warp;
using Data::Maps::WarpItemMatch;

bool HasTag(int itemType, const std::optional<ItemTag>& tag)
{
    return tag.has_value() && g_ItemDatabase.HasTag(itemType, *tag);
}

bool Matches(const EquippedItem& item, const WarpItemMatch& match)
{
    switch (match.kind)
    {
    case WarpItemMatch::Kind::Item:
        return item.type == Data::Items::MakeItemType(match.group, match.number);
    case WarpItemMatch::Kind::Tag:
        return HasTag(item.type, match.tagValue) && !HasTag(item.type, match.exceptTagValue);
    case WarpItemMatch::Kind::Wings:
        return g_ItemDatabase.GetSlot(item.type) == Data::Items::ItemSlot::Wings;
    }
    return false;
}

bool MatchesAny(const EquippedItem& item, const std::vector<WarpItemMatch>& matches)
{
    return std::any_of(matches.begin(), matches.end(),
                       [&](const WarpItemMatch& match) { return Matches(item, match); });
}

std::wstring ToUpper(const wchar_t* text)
{
    std::wstring upper = text;
    _wcsupr(upper.data());
    return upper;
}

// The word after the first `key` in the text, as written (`upperText` is the text in capitals, where `key` is
// searched): the warp /move or the warp word names. Empty when the text has no `key`.
std::wstring_view NamedWord(std::wstring_view chatText, std::wstring_view upperText, std::wstring_view key)
{
    const size_t at = key.empty() ? std::wstring_view::npos : upperText.find(key);
    if (at == std::wstring_view::npos)
    {
        return {};
    }
    size_t start = at + key.size();
    while (start < chatText.size() && std::iswspace(chatText[start]) != 0)
    {
        ++start;
    }
    size_t end = start;
    while (end < chatText.size() && std::iswspace(chatText[end]) == 0)
    {
        ++end;
    }
    return chatText.substr(start, end - start);
}

// True when the warp's shown name or /move name is `word` in any case, as /warp and the M window find a warp.
bool IsNamedBy(const SEASON3B::CMoveCommandData::MOVEINFODATA& warp, std::wstring_view word)
{
    if (word.empty())
    {
        return false;
    }
    const std::wstring text(word);
    return _wcsicmp(text.c_str(), warp._ReqInfo.szMainMapName) == 0 ||
           _wcsicmp(text.c_str(), warp._ReqInfo.szSubMapName) == 0;
}
} // namespace

const Warp* FindWarpRules(const Data::Maps::WarpList& list, int index)
{
    const auto found =
        std::find_if(list.warps.begin(), list.warps.end(), [&](const Warp& warp) { return warp.index == index; });
    return found != list.warps.end() ? &*found : nullptr;
}

int GetRequiredLevel(const Data::Maps::WarpList& list, const Warp* rules, int level, int baseClass)
{
    if (rules != nullptr && rules->noClassDiscount)
    {
        return level;
    }
    for (const Data::Maps::ClassLevelDiscount& discount : list.classLevelDiscounts)
    {
        if (discount.playerClass == baseClass)
        {
            // Cut, as the game always did for the 2/3 of Magic Gladiators, Dark Lords and Rage Fighters. In whole
            // numbers: a float rounds the product of a high level and a fine fraction before the division.
            return static_cast<int>(static_cast<std::int64_t>(level) * discount.numerator / discount.denominator);
        }
    }
    return level;
}

bool IsGensWarp(const Warp* rules, bool nonPvPServer)
{
    return rules != nullptr && rules->gensOnly && !nonPvPServer;
}

bool IsEquipmentAllowed(const Warp& rules, std::span<const EquippedItem> equipment)
{
    const bool hasNeeded =
        rules.needsOneOf.empty() || std::any_of(equipment.begin(), equipment.end(), [&](const EquippedItem& item)
                                                { return item.durability > 0 && MatchesAny(item, rules.needsOneOf); });
    const bool hasForbidden = std::any_of(equipment.begin(), equipment.end(),
                                          [&](const EquippedItem& item) { return MatchesAny(item, rules.notWith); });
    return hasNeeded && !hasForbidden;
}

bool IsWarpAllowed(const Data::Maps::WarpList& list, const SEASON3B::CMoveCommandData::MOVEREQINFO& warp,
                   const WarpCharacter& character, bool nonPvPServer)
{
    const Warp* rules = FindWarpRules(list, warp.index);
    const int requiredLevel = GetRequiredLevel(list, rules, warp.iReqLevel, character.baseClass);
    if (character.level < requiredLevel || character.zen < warp.iReqZen ||
        character.masterLevel < warp.iReqMasterLevel || character.resets < warp.iReqResets ||
        character.pkLevel >= PVP_MURDERER1)
    {
        return false;
    }
    if (IsGensWarp(rules, nonPvPServer) && !character.gensMember)
    {
        return false;
    }
    return rules == nullptr ||
           (!(rules->pvpServersOnly && nonPvPServer) && IsEquipmentAllowed(*rules, character.equipment));
}

std::wstring GetMasterLevelAndResetsText(const SEASON3B::CMoveCommandData::MOVEREQINFO& warp)
{
    std::wstring text;
    const auto add = [&text](const wchar_t* format, int value)
    {
        wchar_t part[64]{};
        std::swprintf(part, std::size(part), format, value);
        if (!text.empty())
        {
            text += L" ";
        }
        text += part;
    };
    if (warp.iReqMasterLevel > 0)
    {
        add(I18N::Game::WarpMasterLevelD, warp.iReqMasterLevel);
    }
    if (warp.iReqResets > 0)
    {
        add(I18N::Game::WarpResetsD, warp.iReqResets);
    }
    return text;
}

std::optional<std::string> CheckChatWarp(const wchar_t* chatText,
                                         const std::list<SEASON3B::CMoveCommandData::MOVEINFODATA*>& warps,
                                         const wchar_t* warpWord, const Data::Maps::WarpList& list,
                                         std::span<const EquippedItem> equipment)
{
    // The text in capitals is searched for "/MOVE" and for the warp word as the
    // game text has it, so a warp word in small letters ("/warp") is never found.
    const std::wstring upperText = ToUpper(chatText);
    if (upperText.find(L"/MOVE") == std::wstring::npos &&
        (wcslen(warpWord) == 0 || upperText.find(warpWord) == std::wstring::npos))
    {
        return std::nullopt;
    }
    // The warp the word after /move or the warp word names (in any case); only when no warp is
    // that word, the first whose name is in the text ("/move Atlans2" is Atlans2,
    // not Atlans, whose name is in it too).
    const std::wstring_view moveWord = NamedWord(chatText, upperText, L"/MOVE");
    const std::wstring_view warpWordWord = NamedWord(chatText, upperText, warpWord);
    auto named = std::find_if(warps.begin(), warps.end(), [&](const SEASON3B::CMoveCommandData::MOVEINFODATA* warp)
                              { return IsNamedBy(*warp, moveWord) || IsNamedBy(*warp, warpWordWord); });
    if (named == warps.end())
    {
        named = std::find_if(warps.begin(), warps.end(),
                             [&](const SEASON3B::CMoveCommandData::MOVEINFODATA* warp)
                             {
                                 // An empty name (one the editor is typing) would be in every text.
                                 const wchar_t* name = warp->_ReqInfo.szMainMapName;
                                 const wchar_t* command = warp->_ReqInfo.szSubMapName;
                                 return (name[0] != L'\0' && wcsstr(chatText, name) != nullptr) ||
                                        (command[0] != L'\0' && upperText.find(ToUpper(command)) != std::wstring::npos);
                             });
    }
    if (named == warps.end())
    {
        return std::nullopt;
    }
    const Warp* rules = FindWarpRules(list, (*named)->_ReqInfo.index);
    // Level, zen and the other rules are the server's to check here; the
    // equipment rules are the M window's.
    if (rules == nullptr || IsEquipmentAllowed(*rules, equipment))
    {
        return std::nullopt;
    }
    return rules->blockedMessage;
}

Equipment MakeEquipment(const tagITEM* equipment, int count)
{
    Equipment items{};
    for (int slot = 0; slot < std::min(count, MAX_EQUIPMENT); ++slot)
    {
        items[slot] = {equipment[slot].Type, equipment[slot].Durability};
    }
    return items;
}

WarpCharacter MakeWarpCharacter(int playerClass, int level, unsigned long zen, int pkLevel, int gensInfluence,
                                int masterLevel, int resets, const tagITEM* equipment, int count)
{
    WarpCharacter character;
    character.baseClass = gCharacterManager.GetBaseClass(static_cast<CLASS_TYPE>(playerClass));
    character.level = level;
    character.zen = static_cast<int>(zen);
    character.pkLevel = pkLevel;
    character.gensMember = gensInfluence != 0;
    character.masterLevel = masterLevel;
    character.resets = resets;
    character.equipment = MakeEquipment(equipment, count);
    return character;
}

WarpCharacter MakeHeroWarpCharacter()
{
    return MakeWarpCharacter(CharacterAttribute->Class, CharacterAttribute->Level, CharacterMachine->Gold, Hero->PK,
                             Hero->m_byGensInfluence, gCharacterManager.GetHeroMasterLevel(),
                             static_cast<int>(CharacterAttribute->Resets), CharacterMachine->Equipment, MAX_EQUIPMENT);
}
} // namespace GameLogic::Warps
