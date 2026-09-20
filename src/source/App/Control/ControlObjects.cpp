#include "stdafx.h"
#include "App/Control/ControlObjects.h"

#include "Core/Text/Utf8.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzObject.h"

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

std::wstring ItemDisplayName(int itemType, int itemLevel)
{
    // The empty marker is -1; type 0 is a real item (the Dark Knight's
    // starting Kris), so it must not be treated as an empty slot.
    if (itemType < 0)
    {
        return {};
    }

    // Longest name the item tables hold, plus room for the terminator.
    wchar_t name[128] = {};
    GetItemName(itemType, itemLevel, name);
    return name;
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
    return {static_cast<int>(object.Position[0] / TERRAIN_SCALE), static_cast<int>(object.Position[1] / TERRAIN_SCALE)};
}
} // namespace App::Control
