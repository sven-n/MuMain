#include "stdafx.h"
#include "App/Control/ControlState.h"

#include "App/Control/ControlObjects.h"
#include "App/Control/ControlTaps.h"
#include "Camera/CameraState.h"

#include "Core/Text/Utf8.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzObject.h"
#include "Core/Utilities/_GlobalFunctions.h"
#include "GameLogic/Items/InventoryUtils.h"
#include "Network/Server/WSclient.h"
#include "Scenes/SceneCore.h"
#include "UI/NewUI/NewUISystem.h"
#include "World/MapInfra/MapManager.h"

#include "json.hpp"

#include <cmath>
#include <utility>

namespace
{
using nlohmann::json;

std::string ItemName(const ITEM& item)
{
    return Core::Text::ToUtf8(App::Control::ItemDisplayName(item.Type, item.Level).c_str());
}

json DescribeItem(const ITEM& item, int slot)
{
    json described;
    described["slot"] = slot;
    described["name"] = ItemName(item);
    described["level"] = item.Level;
    described["durability"] = item.Durability;
    // In inventory squares, so a script can tell whether an item fits somewhere.
    described["width"] = ItemAttribute[item.Type].Width;
    described["height"] = ItemAttribute[item.Type].Height;
    return described;
}

// The items of a trade grid, by the grid's own slot numbers.
json TradeGridItems(SEASON3B::CNewUIInventoryCtrl* grid)
{
    json items = json::array();
    if (grid == nullptr)
    {
        return items;
    }
    for (int i = 0; i < static_cast<int>(grid->GetNumberOfItems()); ++i)
    {
        const ITEM* item = grid->GetItem(i);
        if (item != nullptr)
        {
            items.push_back(DescribeItem(*item, item->y * grid->GetNumberOfColumn() + item->x));
        }
    }
    return items;
}

// The open trade, or null: the partner, both offers and both confirm buttons.
json TradeState()
{
    if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE))
    {
        return nullptr;
    }

    wchar_t partner[MAX_USERNAME_SIZE + 1]{};
    g_pTrade->GetYourID(partner);
    json trade;
    trade["partner"] = Core::Text::ToUtf8(partner);
    trade["partner_level"] = g_pTrade->GetYourLevel();
    trade["my_items"] = TradeGridItems(g_pTrade->GetMyInvenCtrl());
    trade["partner_items"] = TradeGridItems(g_pTrade->GetYourInvenCtrl());
    trade["my_zen"] = g_pTrade->GetMyTradeGold();
    trade["partner_zen"] = g_pTrade->GetYourTradeGold();
    trade["my_confirmed"] = g_pTrade->IsMyConfirmed();
    // Frames until the confirm button takes clicks again after an offer changed.
    trade["my_confirm_wait"] = g_pTrade->GetMyTradeWait();
    trade["partner_confirmed"] = g_pTrade->IsYourConfirmed();
    return trade;
}

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
    case KIND_PET:
        return "pet";
    default:
        return "unknown";
    }
}

json EquipmentArray()
{
    json equipment = json::array();
    if (CharacterMachine == nullptr)
    {
        return equipment;
    }

    for (int slot = 0; slot < MAX_EQUIPMENT; ++slot)
    {
        const ITEM& item = CharacterMachine->Equipment[slot];
        // The empty marker is -1; type 0 is a real item (ITEM_KRIS, the Dark
        // Knight's starting weapon), so it must not be skipped here.
        if (item.Type < 0)
        {
            continue;
        }
        equipment.push_back(DescribeItem(item, slot));
    }
    return equipment;
}

json InventoryArray()
{
    json inventory = json::array();
    for (int slot = MAX_EQUIPMENT_INDEX; slot < MAX_MY_INVENTORY_EX_INDEX; ++slot)
    {
        const ITEM* item = FindInventoryItemBySlot(slot);
        if (item == nullptr || item->Type < 0)
        {
            continue;
        }
        inventory.push_back(DescribeItem(*item, slot));
    }
    return inventory;
}

json PartyArray()
{
    json members = json::array();
    for (int index = 0; index < MAX_PARTYS; ++index)
    {
        const PARTY_t& member = Party[index];
        if (member.Name[0] == L'\0')
        {
            continue;
        }

        json described;
        described["name"] = Core::Text::ToUtf8(member.Name);
        described["map"] = member.Map;
        described["position"] = json::array({member.x, member.y});
        described["hp"] = member.currHP;
        described["max_hp"] = member.maxHP;
        members.push_back(std::move(described));
    }
    return members;
}

// The skills the character actually has, by number, so a scenario can pick
// one instead of guessing.
json SkillArray()
{
    json skills = json::array();
    if (CharacterAttribute == nullptr)
    {
        return skills;
    }

    for (int index = 0; index < MAX_MAGIC; ++index)
    {
        const ActionSkillType skill = CharacterAttribute->Skill[index];
        if (skill != AT_SKILL_UNDEFINED)
        {
            skills.push_back(static_cast<int>(skill));
        }
    }
    return skills;
}

json BuffArray()
{
    json buffs = json::array();
    if (Hero == nullptr)
    {
        return buffs;
    }

    // The client keeps the active buffs in the object's own buff map;
    // report their numeric states, which is what it knows without the UI.
    OBJECT* object = &Hero->Object;
    const DWORD count = g_CharacterBuffSize(object);
    for (DWORD index = 0; index < count; ++index)
    {
        buffs.push_back(static_cast<int>(g_CharacterBuff(object, static_cast<int>(index))));
    }
    return buffs;
}
// The world camera of the last frame, see App::Control::Frames::RecordWorldCamera.
CameraState g_worldCamera;
bool g_hasWorldCamera = false;

// Where a character is drawn, in window pixels: the middle of the box the
// mouse picks it by (Input/Selection.cpp), projected as the mouse ray is cast
// (CameraProjection::ScreenToWorldRay), so `click-ui` there points at it. Null
// while it is not drawn, behind the camera or outside the window.
json ObjectPixel(const OBJECT& object)
{
    if (!g_hasWorldCamera || !object.Visible)
    {
        return nullptr;
    }

    const OBB_t& box = object.OBB;
    vec3_t center;
    for (int axis = 0; axis < 3; ++axis)
    {
        center[axis] = box.StartPos[axis] + (box.XAxis[axis] + box.YAxis[axis] + box.ZAxis[axis]) * 0.5f;
    }

    vec3_t camera;
    VectorTransform(center, g_worldCamera.Matrix, camera);
    if (camera[2] >= 0.0f)
    {
        return nullptr;
    }

    const float x =
        static_cast<float>(g_worldCamera.ScreenCenterX) - camera[0] / (g_worldCamera.PerspectiveX * camera[2]);
    const float y =
        static_cast<float>(g_worldCamera.ScreenCenterY) + camera[1] / (g_worldCamera.PerspectiveY * camera[2]);
    if (x < 0.0f || y < 0.0f || x >= static_cast<float>(WindowWidth) || y >= static_cast<float>(WindowHeight))
    {
        return nullptr;
    }

    json pixel;
    pixel["x"] = std::round(x);
    pixel["y"] = std::round(y);
    return pixel;
}
} // namespace

namespace App::Control
{
std::string WorldStateObject()
{
    json state;
    state["scene"] = "world";
    state["account"] = Core::Text::ToUtf8(LogInID);

    if (Hero == nullptr || CharacterAttribute == nullptr)
    {
        return state.dump();
    }

    state["character"] = Core::Text::ToUtf8(CharacterAttribute->Name);
    state["class"] = static_cast<int>(CharacterAttribute->Class);
    state["level"] = CharacterAttribute->Level;
    state["experience"] = CharacterAttribute->Experience;
    state["next_experience"] = CharacterAttribute->NextExperience;
    state["zen"] = CharacterMachine != nullptr ? CharacterMachine->Gold : 0;

    state["hp"] = CharacterAttribute->Life;
    state["max_hp"] = CharacterAttribute->LifeMax;
    state["mana"] = CharacterAttribute->Mana;
    state["max_mana"] = CharacterAttribute->ManaMax;
    state["sd"] = CharacterAttribute->Shield;
    state["max_sd"] = CharacterAttribute->ShieldMax;
    state["ag"] = CharacterAttribute->SkillMana;
    state["max_ag"] = CharacterAttribute->SkillManaMax;

    state["map"] = gMapManager.WorldActive;
    state["map_name"] = Core::Text::ToUtf8(gMapManager.GetMapName(gMapManager.WorldActive));
    state["position"] = json::array({Hero->PositionX, Hero->PositionY});
    state["alive"] = Hero->Dead <= 0;
    // Attacks are refused inside a safe zone; a scenario needs to know.
    state["safe_zone"] = Hero->SafeZone;
    state["id"] = HeroKey;

    const bool hasTarget = SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT &&
                           CharactersClient[SelectedCharacter].Object.Live;
    state["target"] = hasTarget ? json(CharactersClient[SelectedCharacter].Key) : json(nullptr);

    state["equipment"] = EquipmentArray();
    state["inventory"] = InventoryArray();
    state["skills"] = SkillArray();
    state["buffs"] = BuffArray();
    state["party"] = PartyArray();
    state["trade"] = TradeState();
    state["nearby"] = NearbyArray();

    return state.dump();
}

json NearbyArray()
{
    json nearby = json::array();

    for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
    {
        const CHARACTER& character = CharactersClient[index];
        if (!character.Object.Live || character.Key == HeroKey)
        {
            continue;
        }

        json described;
        described["id"] = character.Key;
        described["kind"] = ObjectKindName(character.Object.Kind);
        described["name"] = Core::Text::ToUtf8(character.ID);
        described["position"] = json::array({character.PositionX, character.PositionY});
        described["alive"] = character.Dead <= 0;
        described["level"] = character.Level;
        // The client only knows another object's health as the fraction the
        // server sends in 1/250ths (`HealthStatus`), and as -1 while it has
        // not been told at all. Report it as a percentage, because that is
        // what the field is called, and as null when it is unknown: a number
        // there would compare below every threshold a scenario writes.
        // Recovered from the packet's own 1/250th before scaling, so the
        // number reads as the server meant it (95.2, not 95.20000457763672,
        // which is what the `float` division left behind).
        const double healthPercent = std::round(character.HealthStatus * 250.0f) * 100.0 / 250.0;
        described["hp_percent"] = character.HealthStatus < 0.0f ? json(nullptr) : json(healthPercent);
        described["pixel"] = ObjectPixel(character.Object);
        nearby.push_back(std::move(described));
    }

    for (int index = 0; index < MAX_ITEMS; ++index)
    {
        const ITEM_t& drop = Items[index];
        if (drop.Item.Type < 0 || !drop.Object.Live)
        {
            continue;
        }

        json described;
        // Drops are addressed by the client's own slot in the dropped-item
        // table: that is what the pickup packet carries.
        described["id"] = index;
        described["kind"] = "item";
        described["name"] = ItemName(drop.Item);
        const std::pair<int, int> tile = App::Control::DropTile(index);
        described["position"] = json::array({tile.first, tile.second});
        nearby.push_back(std::move(described));
    }

    return nearby;
}
} // namespace App::Control

namespace App::Control::Frames
{
void RecordWorldCamera()
{
    g_worldCamera = g_Camera;
    g_hasWorldCamera = true;
}
} // namespace App::Control::Frames
