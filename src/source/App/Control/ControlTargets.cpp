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
// The nearest living monster or NPC with this monster number, by tiles from
// the hero; the first one in the character table on a tie. Summons and
// transformed players carry a monster number too, but are players.
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
            character.Object.Kind == KIND_PLAYER)
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
    const std::string& text = fields.at("monster");
    int parsed = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), parsed);
    if (error != std::errc{} || end != text.data() + text.size() || parsed < 0 ||
        parsed >= Data::Monsters::MonsterNumberCount)
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
