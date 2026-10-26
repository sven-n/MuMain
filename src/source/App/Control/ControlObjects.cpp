#include "stdafx.h"
#include "App/Control/ControlObjects.h"

#include "Core/Text/Utf8.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzObject.h"

namespace App::Control
{
std::string_view ObjectKindName(int objectKind)
{
    switch (objectKind)
    {
    case KIND_PLAYER:
        return "player";
    case KIND_MONSTER:
        return "monster";
    case KIND_NPC:
        return "npc";
    case KIND_TRAP:
        return "trap";
    case KIND_PET:
        return "pet";
    case KIND_TMP:
        return "other";
    default:
        return "unknown";
    }
}

std::optional<int> MonsterNumberOf(const CHARACTER& character)
{
    if (character.MonsterIndex == MONSTER_UNDEFINED)
    {
        return std::nullopt;
    }
    return static_cast<int>(character.MonsterIndex);
}

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
    description.kind = ObjectKindName(character.Object.Kind);
    description.monster = MonsterNumberOf(character);
    description.x = character.PositionX;
    description.y = character.PositionY;
    description.known = true;
    return description;
}

std::wstring ItemDisplayName(int itemType, int itemLevel)
{
    return GetItemName(itemType, itemLevel);
}

std::wstring DropName(int itemSlot)
{
    if (itemSlot < 0 || itemSlot >= MAX_ITEMS)
    {
        return {};
    }

    const ITEM& item = Items[itemSlot].Item;
    return ItemDisplayName(item.Type, item.Level);
}

std::pair<int, int> DropTile(int itemSlot)
{
    if (itemSlot < 0 || itemSlot >= MAX_ITEMS)
    {
        return {-1, -1};
    }

    const OBJECT& object = Items[itemSlot].Object;
    if (object.Position[0] < 0.0f || object.Position[1] < 0.0f)
    {
        // Truncation towards zero would turn a position off the map into
        // tile 0, which reads as a real place; the header's "no tile" answer
        // is what this is.
        return {-1, -1};
    }
    return {static_cast<int>(object.Position[0] / TERRAIN_SCALE), static_cast<int>(object.Position[1] / TERRAIN_SCALE)};
}
} // namespace App::Control
