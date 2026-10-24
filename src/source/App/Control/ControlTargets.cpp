#include "stdafx.h"
#include "App/Control/ControlTargets.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Engine/Object/ZzzCharacter.h"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <cwchar>

namespace App::Control::Targets
{
namespace
{
// Traps and objects of the kind other carry a monster number too, and so do
// summons, Kalima gates and transformed players, which are players; only
// monsters and NPCs are found by it.
bool IsMonsterOrNpc(const OBJECT& object)
{
    return object.Kind == KIND_MONSTER || object.Kind == KIND_NPC;
}

// The nearest living monster or NPC with this monster number, by tiles from
// the hero; the first one in the character table on a tie.
bool FindNearestMonster(int number, int& key)
{
    if (CharactersClient == nullptr)
    {
        return false;
    }
    int bestDistance = -1;
    for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
    {
        const CHARACTER& character = CharactersClient[index];
        if (!character.Object.Live || character.Dead > 0 || character.MonsterIndex != number ||
            !IsMonsterOrNpc(character.Object))
        {
            continue;
        }
        const int distance = Hero == nullptr ? 0
                                             : std::max(std::abs(character.PositionX - Hero->PositionX),
                                                        std::abs(character.PositionY - Hero->PositionY));
        if (bestDistance < 0 || distance < bestDistance)
        {
            bestDistance = distance;
            key = character.Key;
        }
    }
    return bestDistance >= 0;
}
} // namespace

bool ReadMonsterNumber(const std::map<std::string, std::string>& fields, int& number)
{
    if (fields.size() != 1 || !fields.contains("monster"))
    {
        return false;
    }
    // The fields hold a JSON number as written out (251, or 251.0 for a
    // float) and a JSON string as it is, so "251" counts as well.
    const std::string& text = fields.at("monster");
    const char* const last = text.data() + text.size();
    int parsed = 0;
    auto [end, error] = std::from_chars(text.data(), last, parsed);
    if (error != std::errc{})
    {
        return false;
    }
    // A whole number written as a float: only zeros after the point.
    if (end != last && *end == '.' && end + 1 != last)
    {
        end = std::find_if(end + 1, last, [](char digit) { return digit != '0'; });
    }
    if (end != last || parsed < 0 || parsed >= Data::Monsters::MonsterNumberCount)
    {
        return false;
    }
    number = parsed;
    return true;
}

TargetLookup ResolveTarget(const Request& request, int& key)
{
    key = -1;
    if (!request.Has("target"))
    {
        return TargetLookup::Missing;
    }

    int id = 0;
    if (request.GetInt("target", id))
    {
        if (FindCharacterIndex(id) >= MAX_CHARACTERS_CLIENT)
        {
            return TargetLookup::NotInView;
        }
        key = id;
        return TargetLookup::Found;
    }

    std::map<std::string, std::string> fields;
    if (request.GetStringMap("target", fields))
    {
        int number = 0;
        if (!ReadMonsterNumber(fields, number))
        {
            return TargetLookup::Malformed;
        }
        return FindNearestMonster(number, key) ? TargetLookup::Found : TargetLookup::NotInView;
    }

    std::string name;
    if (!request.GetString("target", name) || name.empty())
    {
        return TargetLookup::Malformed;
    }

    const std::wstring wanted = Core::Text::FromUtf8(name);
    for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
    {
        const CHARACTER& character = CharactersClient[index];
        if (character.Object.Live && wcscmp(character.ID, wanted.c_str()) == 0)
        {
            key = character.Key;
            return TargetLookup::Found;
        }
    }
    return TargetLookup::NotInView;
}

std::string TargetError(const Request& request, TargetLookup lookup, std::string_view command, bool required)
{
    switch (lookup)
    {
    case TargetLookup::Found:
        return {};
    case TargetLookup::Missing:
        return required ? App::Control::EncodeError(request.EncodedId(), App::Control::ErrorCode::BadRequest,
                                                    "`" + std::string(command) + "` needs a `target`")
                        : std::string{};
    case TargetLookup::Malformed:
        return App::Control::EncodeError(request.EncodedId(), App::Control::ErrorCode::BadRequest,
                                         "`target` is an object id, a character name or {\"monster\": <number>}");
    case TargetLookup::NotInView:
        break;
    }
    return App::Control::EncodeError(request.EncodedId(), App::Control::ErrorCode::NotInView,
                                     "the client does not see that object");
}
} // namespace App::Control::Targets
