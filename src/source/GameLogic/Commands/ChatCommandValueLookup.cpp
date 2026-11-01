#include "stdafx.h"
#include "GameLogic/Commands/ChatCommandValueLookup.h"

#include "I18N/All.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Guild/GuildCache.h"
#include "World/MapInfra/MapManager.h"

#include <cerrno>
#include <climits>
#include <cwchar>

namespace GameLogic::Commands::ValueLookup
{
namespace
{
// What getMonsterName answers for a monster it doesn't know.
constexpr const wchar_t* UnknownMonsterName = L"()";

bool TryParseNumber(const std::wstring& text, int& number)
{
    wchar_t* end = nullptr;
    errno = 0;
    const auto parsed = std::wcstol(text.c_str(), &end, 10);
    if (errno == ERANGE || end == text.c_str() || *end != L'\0' || parsed < INT_MIN || parsed > INT_MAX)
    {
        return false;
    }

    number = static_cast<int>(parsed);
    return true;
}

std::wstring GetMapName(int map)
{
    if (map < 0 || map >= NUM_WD)
    {
        return {};
    }

    const auto* name = gMapManager.GetMapName(map);
    return name != nullptr ? name : std::wstring();
}

std::wstring GetMonsterName(int number)
{
    const auto* name = getMonsterName(number);
    if (name == nullptr || std::wcscmp(name, UnknownMonsterName) == 0)
    {
        return {};
    }

    return name;
}

std::wstring GetItemName(int group, int number)
{
    if (ItemAttribute == nullptr || group < 0 || group >= MAX_ITEM_TYPE || number < 0 || number >= MAX_ITEM_INDEX)
    {
        return {};
    }

    return ItemAttribute[group * MAX_ITEM_INDEX + number].Name;
}

std::wstring GetSkillName(int number)
{
    if (SkillAttribute == nullptr || number < 0 || number >= MAX_SKILLS)
    {
        return {};
    }

    return SkillAttribute[number].Name;
}

// The id of an object is only known while it's in the scope of the character.
std::wstring GetObjectName(int id)
{
    const auto* character = FindCharacterByKey(id);
    return character != nullptr ? character->ID : std::wstring();
}

std::wstring GetItemNameOfNumber(const ChatCommandParameter& parameter, int number,
                                 const std::vector<std::wstring>& values)
{
    // The number alone doesn't identify an item, its group is needed as well.
    const auto groupIndex = parameter.GroupWithIndex;
    if (groupIndex == ChatCommandParameter::NoGroup || static_cast<size_t>(groupIndex) >= values.size())
    {
        return {};
    }

    int group = 0;
    if (!TryParseNumber(values[groupIndex], group))
    {
        return {};
    }

    return GetItemName(group, number);
}

std::wstring GetOwnGuildName()
{
    if (Hero->GuildMarkIndex < 0 || Hero->GuildMarkIndex >= MAX_MARKS)
    {
        return {};
    }

    return GuildMark[Hero->GuildMarkIndex].GuildName;
}

// The codes of the locales are plain ASCII.
std::wstring GetOwnLanguageCode()
{
    const std::string locale = I18N::GetCurrentLocale();
    return std::wstring(locale.begin(), locale.end());
}
} // namespace

bool HasOwnValue(const ChatCommandParameter& parameter)
{
    switch (parameter.ValueReference)
    {
    case ChatCommandValueReference::CharacterName:
    case ChatCommandValueReference::GuildName:
    case ChatCommandValueReference::Map:
    case ChatCommandValueReference::MapCoordinateX:
    case ChatCommandValueReference::MapCoordinateY:
    case ChatCommandValueReference::LanguageIsoCode:
        return true;

    default:
        return false;
    }
}

std::wstring GetOwnValue(const ChatCommandParameter& parameter)
{
    if (Hero == nullptr)
    {
        return {};
    }

    switch (parameter.ValueReference)
    {
    case ChatCommandValueReference::CharacterName:
        return Hero->ID;

    case ChatCommandValueReference::GuildName:
        return GetOwnGuildName();

    case ChatCommandValueReference::Map:
        return std::to_wstring(gMapManager.WorldActive);

    case ChatCommandValueReference::MapCoordinateX:
        return std::to_wstring(Hero->PositionX);

    case ChatCommandValueReference::MapCoordinateY:
        return std::to_wstring(Hero->PositionY);

    case ChatCommandValueReference::LanguageIsoCode:
        return GetOwnLanguageCode();

    default:
        return {};
    }
}

std::wstring DescribeValue(const ChatCommand& command, size_t parameterIndex, const std::vector<std::wstring>& values)
{
    if (parameterIndex >= command.Parameters.size() || parameterIndex >= values.size())
    {
        return {};
    }

    const auto& parameter = command.Parameters[parameterIndex];
    int number = 0;
    if (!TryParseNumber(values[parameterIndex], number))
    {
        return {};
    }

    switch (parameter.ValueReference)
    {
    case ChatCommandValueReference::Map:
        return GetMapName(number);

    case ChatCommandValueReference::MonsterNumber:
        return GetMonsterName(number);

    case ChatCommandValueReference::ItemNumber:
        return GetItemNameOfNumber(parameter, number, values);

    case ChatCommandValueReference::SkillNumber:
        return GetSkillName(number);

    case ChatCommandValueReference::ObjectId:
        return GetObjectName(number);

    default:
        return {};
    }
}
} // namespace GameLogic::Commands::ValueLookup
