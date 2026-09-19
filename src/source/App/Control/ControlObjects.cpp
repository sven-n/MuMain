#include "stdafx.h"
#include "App/Control/ControlObjects.h"

#include "Core/Text/Utf8.h"
#include "Engine/Object/ZzzCharacter.h"

namespace
{
std::string KindName(int objectKind)
{
    switch (objectKind)
    {
    case KIND_PLAYER:
        return "player";
    case KIND_MONSTER:
        return "monster";
    case KIND_NPC:
        return "npc";
    case KIND_PET:
        return "pet";
    default:
        return "unknown";
    }
}
} // namespace

namespace App::Control
{
Events::ObjectDescription DescribeGameObject(int key)
{
    Events::ObjectDescription description;
    description.id = key;

    if (key < 0 || CharactersClient == nullptr)
    {
        return description;
    }

    const int index = FindCharacterIndex(key);
    if (index == MAX_CHARACTERS_CLIENT)
    {
        return description;
    }

    const CHARACTER& character = CharactersClient[index];
    description.name = Core::Text::ToUtf8(character.ID);
    description.kind = KindName(character.Object.Kind);
    description.x = character.PositionX;
    description.y = character.PositionY;
    description.known = true;
    return description;
}
} // namespace App::Control
