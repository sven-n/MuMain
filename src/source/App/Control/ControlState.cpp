#include "stdafx.h"
#include "App/Control/ControlState.h"

#include "App/Control/ControlObjects.h"
#include "App/Control/ControlTaps.h"
#include "Camera/CameraProjection.h"
#include "Camera/CameraState.h"

#include "Core/Text/Utf8.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzObject.h"
#include "Core/Utilities/_GlobalFunctions.h"
#include "UI/Inventory/InventoryContents.h"
#include "GameLogic/Items/InventoryUtils.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Quests/CSQuest.h"
#include "GameLogic/Quests/DialogStructure.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/Items/ShopRestrictions.h"
#include "Network/Server/WSclient.h"
#include "Scenes/SceneCore.h"
#include "UI/Core/WindowSystem.h"
#include "World/MapInfra/MapManager.h"

#include "json.hpp"

#include <cmath>
#include <utility>

// The quest dialog's page, text and answers (Scenes/SceneCore.cpp).
extern int g_iNumLineMessageBoxCustom;
extern wchar_t g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
extern wchar_t g_lpszDialogAnswer[MAX_ANSWER_FOR_DIALOG][NUM_LINE_DA][MAX_LENGTH_CMB];
extern int g_iCurrentDialogScript;

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
    // What a repair brings the durability back to (a stack's count for potions
    // and jewels, which are not repaired).
    described["max_durability"] = CalcMaxDurability(&item, &ItemAttribute[item.Type], item.Level);
    // In inventory squares, so a script can tell whether an item fits somewhere.
    described["width"] = ItemAttribute[item.Type].Width;
    described["height"] = ItemAttribute[item.Type].Height;
    return described;
}

// What repairing the item costs, as its tooltip shows while a click would
// repair it: at an NPC that repairs, or in the inventory's repair mode (which
// costs more). Left out for an item at full durability or one that is never
// repaired.
void AddRepairPrice(json& described, const ITEM& item)
{
    const bool atNpc = g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop();
    const bool selfRepair = g_pMyInventory->GetRepairMode() == SEASON3B::REPAIR_MODE_ON;
    ITEM* repaired = const_cast<ITEM*>(&item);
    if ((!atNpc && !selfRepair) || GameLogic::Items::IsRepairBan(repaired))
    {
        return;
    }

    const int maxDurability = CalcMaxDurability(&item, &ItemAttribute[item.Type], item.Level);
    if (item.Durability >= maxDurability)
    {
        return;
    }

    // No value yet (a Dark Lord pet whose data has not arrived): the tooltip
    // shows no price either.
    const int64_t value = ItemValue(repaired, 2);
    if (value < 0)
    {
        return;
    }

    // The tooltip's own function, which picks the NPC or the self-repair price.
    wchar_t text[100] = {};
    described["repair_price"] = ConvertRepairGold(value, item.Durability, maxDurability, item.Type, text);
}

// The items of a trade grid, by the grid's own slot numbers.
json TradeGridItems(mu::ui::window::CInventoryCtrl* grid)
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
    if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_TRADE))
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
        json described = DescribeItem(item, slot);
        AddRepairPrice(described, item);
        equipment.push_back(std::move(described));
    }
    return equipment;
}

json InventoryArray()
{
    json inventory = json::array();
    for (int slot = MAX_EQUIPMENT_INDEX; slot < MAX_MY_INVENTORY_EX_INDEX; ++slot)
    {
        const ITEM* item = UI::Inventory::FindPlayerItem(slot);
        if (item == nullptr || item->Type < 0)
        {
            continue;
        }
        json described = DescribeItem(*item, slot);
        // While an NPC shop is open, what it pays for the item, as its tooltip shows.
        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCSHOP))
        {
            described["sell_price"] = ItemValue(const_cast<ITEM*>(item), 1);
        }
        AddRepairPrice(described, *item);
        inventory.push_back(std::move(described));
    }
    return inventory;
}

// The open NPC shop: whether it repairs, and its goods with the price its
// tooltip shows, tax included; null while no shop is open.
json NpcShopState()
{
    if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCSHOP))
    {
        return nullptr;
    }

    json shop;
    shop["repair_shop"] = g_pNPCShop->IsRepairShop();
    shop["tax_rate"] = g_pNPCShop->GetTaxRate();
    // What "Repair all" costs, as the shop shows it (worked out every frame).
    if (g_pNPCShop->IsRepairShop())
    {
        shop["repair_all_price"] = AllRepairGold;
    }
    json items = json::array();
    mu::ui::window::CInventoryCtrl* grid = g_pNPCShop->GetInventoryCtrl();
    for (int i = 0; grid != nullptr && i < static_cast<int>(grid->GetNumberOfItems()); ++i)
    {
        ITEM* item = grid->GetItem(i);
        if (item == nullptr)
        {
            continue;
        }
        json described = DescribeItem(*item, item->y * grid->GetNumberOfColumn() + item->x);
        const int64_t price = ItemValue(item, 0);
        described["price"] = price + price * g_pNPCShop->GetTaxRate() / 100;
        items.push_back(std::move(described));
    }
    shop["items"] = std::move(items);
    return shop;
}

// The goods of a personal shop grid with their prices, by the slot numbers
// the server uses (204 and up); `priceTable` is PSHOPWNDTYPE_SALE for the
// player's own shop, PSHOPWNDTYPE_PURCHASE for the one it visits.
json PersonalShopItems(mu::ui::window::CInventoryCtrl* grid, int priceTable)
{
    json items = json::array();
    for (int i = 0; grid != nullptr && i < static_cast<int>(grid->GetNumberOfItems()); ++i)
    {
        ITEM* item = grid->GetItem(i);
        if (item == nullptr)
        {
            continue;
        }
        const int slot = grid->GetIndexByItem(item);
        json described = DescribeItem(*item, slot);
        int price = 0;
        described["price"] = GetPersonalItemPrice(slot, price, priceTable) ? json(price) : json(nullptr);
        items.push_back(std::move(described));
    }
    return items;
}

// The player's own personal shop: whether it is open to others, the title in
// its title field, and its goods with their prices.
json MyShopState()
{
    json shop;
    shop["open"] = g_pMyShopInventory->IsEnablePersonalShop();
    wchar_t title[MAX_SHOPTITLE + 1] = {};
    g_pMyShopInventory->GetTitle(title);
    shop["title"] = Core::Text::ToUtf8(title);
    shop["items"] = PersonalShopItems(g_pMyShopInventory->GetInventoryCtrl(), PSHOPWNDTYPE_SALE);
    return shop;
}

// The personal shop the player looks into: whose it is, its title and its
// goods with their prices; null while none is open.
json PurchaseShopState()
{
    if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_PURCHASESHOP_INVENTORY))
    {
        return nullptr;
    }

    json shop;
    const int seller = g_pPurchaseShopInventory->GetShopCharacterIndex();
    shop["seller"] =
        seller >= 0 && seller < MAX_CHARACTERS_CLIENT ? Core::Text::ToUtf8(CharactersClient[seller].ID) : "";
    shop["title"] = Core::Text::ToUtf8(g_pPurchaseShopInventory->GetTitleText().c_str());
    shop["items"] = PersonalShopItems(g_pPurchaseShopInventory->GetInventoryCtrl(), PSHOPWNDTYPE_PURCHASE);
    return shop;
}

// The legacy quests (0 and 1 the second class, 2 and 3 Marlon's, 4 to 6 the
// third class) with their state as the server told the client.
json LegacyQuestArray()
{
    constexpr int LegacyQuestCount = 7;
    json quests = json::array();
    for (int index = 0; index < LegacyQuestCount; ++index)
    {
        json quest;
        quest["index"] = index;
        const wchar_t* title = g_csQuest.getQuestTitle(static_cast<BYTE>(index));
        quest["name"] = title != nullptr ? Core::Text::ToUtf8(title) : "";
        switch (g_csQuest.getQuestState2(index))
        {
        case QUEST_ING:
            quest["state"] = "active";
            break;
        case QUEST_END:
            quest["state"] = "complete";
            break;
        case QUEST_NO:
            quest["state"] = "not_started";
            break;
        case QUEST_NONE:
            quest["state"] = "none";
            break;
        default:
            // QUEST_READY and QUEST_ERROR; the events name them the same.
            quest["state"] = "unknown";
            break;
        }
        quests.push_back(std::move(quest));
    }
    return quests;
}

// The quest dialog on screen: the quest, the page, its text and the answers
// a click acts on; null while it is closed.
json NpcQuestState()
{
    if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCQUEST))
    {
        return nullptr;
    }

    json dialog;
    dialog["quest"] = g_csQuest.GetCurrQuestIndex();
    dialog["page"] = g_iCurrentDialogScript;
    std::wstring text;
    for (int line = 0; line < g_iNumLineMessageBoxCustom; ++line)
    {
        text += (line > 0 ? L" " : L"") + std::wstring(g_lpszMessageBoxCustom[line]);
    }
    dialog["text"] = Core::Text::ToUtf8(text.c_str());
    // Each answer with what a click on it does: `next` shows another page,
    // `accept` starts the quest, `complete` hands it in, `close` ends the talk.
    json answers = json::array();
    const auto& entry = GameLogic::Quests::Dialog::GetEntry(g_iCurrentDialogScript);
    for (int answer = 0; answer < entry.numAnswer; ++answer)
    {
        json described;
        described["text"] = Core::Text::ToUtf8(g_lpszDialogAnswer[answer][0]);
        switch (entry.answers[answer].returnCode)
        {
        case 1:
            described["action"] = "accept";
            break;
        case 2:
            described["action"] = "close";
            break;
        case 3:
            described["action"] = "complete";
            break;
        default:
            described["action"] = entry.answers[answer].link > 0 ? "next" : "none";
            break;
        }
        answers.push_back(std::move(described));
    }
    dialog["answers"] = std::move(answers);
    dialog["need_zen"] = g_csQuest.GetNeedZen();
    return dialog;
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

    float x = 0.0f;
    float y = 0.0f;
    if (!CameraProjection::WorldToWindowPixel(g_worldCamera, center, &x, &y))
    {
        return nullptr;
    }

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
    state["class_name"] = Core::Text::ToUtf8(gCharacterManager.GetCharacterClassText(CharacterAttribute->Class));
    state["level"] = CharacterAttribute->Level;
    state["experience"] = CharacterAttribute->Experience;
    state["next_experience"] = CharacterAttribute->NextExperience;
    state["zen"] = CharacterMachine != nullptr ? CharacterMachine->Gold : 0;
    // Points the player can add to a stat with the character window's "+" buttons.
    state["level_up_points"] = CharacterAttribute->LevelUpPoint;
    state["stats"] = {{"strength", CharacterAttribute->Strength},
                      {"agility", CharacterAttribute->Dexterity},
                      {"vitality", CharacterAttribute->Vitality},
                      {"energy", CharacterAttribute->Energy},
                      {"command", CharacterAttribute->Charisma}};
    // The Blade Knight's combo, from Marlon's "Secret of Dark Stone".
    state["combo"] = Hero->byExtensionSkill == 1;

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
    state["npc_shop"] = NpcShopState();
    state["my_shop"] = MyShopState();
    state["purchase_shop"] = PurchaseShopState();
    // Clicks repair items instead of picking them up (the inventory's repair
    // button, `L`, or an NPC's repair button).
    state["repair_mode"] = g_pMyInventory->GetRepairMode() == SEASON3B::REPAIR_MODE_ON;
    state["quests"] = LegacyQuestArray();
    state["npc_quest"] = NpcQuestState();
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
