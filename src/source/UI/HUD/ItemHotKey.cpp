#include "stdafx.h"
#include "UI/HUD/ItemHotKey.h"
#include <algorithm>
#include "I18N/All.h"

#include "UI/Options/OptionWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Widgets/UIBaseDef.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"

#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "World/MapInfra/MapManager.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Skills/SkillManager.h"
#include "UI/HUD/Skills/SkillIconAtlas.h"
#include "UI/HUD/Skills/SkillTooltip.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Time/CTimCheck.h"
#include "GameLogic/Social/MonkSystem.h"
#include "GameLogic/Items/ItemCategories.h"
#include "Data/GameData/ItemData/ItemDatabase.h"

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "GameShop/InGameShopSystem.h"
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Renderer/MuRenderer.h"
#include "Camera/CameraProjection.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlWorkspaceParticipant.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include "GameLogic/Quests/QuestMng.h"
#include "UI/Social/FriendWindow.h"

mu::ui::window::CItemHotKey::CItemHotKey()
{
    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        m_iHotKeyItemType[i] = -1;
        m_iHotKeyItemLevel[i] = 0;
        m_SlotTargets[i] = std::make_unique<UI::Items::ItemCameraTarget>(
            [this, i](const Rml::Vector2f& offset, const Rml::Vector2f& size) { RenderSlot(i, offset, size); });
    }
}

mu::ui::window::CItemHotKey::~CItemHotKey()
{
}

bool mu::ui::window::CItemHotKey::UpdateKeyEvent()
{
    int iIndex = -1;

    if (mu::ui::window::IsPress('Q') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_Q);
    }
    else if (mu::ui::window::IsPress('W') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_W);
    }
    else if (mu::ui::window::IsPress('E') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_E);
    }
    else if (mu::ui::window::IsPress('R') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_R);
    }

    if (iIndex != -1)
    {
        ITEM* pItem = NULL;
        pItem = g_pMyInventory->FindItem(iIndex);
        if (GameLogic::Items::IsElixir(pItem))
        {
            std::list<eBuffState> secretPotionbufflist;
            secretPotionbufflist.push_back(eBuff_SecretPotion1);
            secretPotionbufflist.push_back(eBuff_SecretPotion2);
            secretPotionbufflist.push_back(eBuff_SecretPotion3);
            secretPotionbufflist.push_back(eBuff_SecretPotion4);
            secretPotionbufflist.push_back(eBuff_SecretPotion5);

            if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) != eBuffNone) {
                mu::ui::window::CreateOkMessageBox(I18N::Game::YouCannotUseThisItemWhileThePotionEffectsRemainActive, RGBA(255, 30, 0, 255));
            }
            else {
                SendRequestUse(iIndex, 0);
            }
        }
        else

        {
            SendRequestUse(iIndex, 0);
        }
        return false;
    }

    return true;
}

namespace
{
// Items a hotkey takes only of the type it is bound to; Remedy of Love only at level 0.
bool IsSingleHotKeyItem(int itemType, int itemLevel)
{
    switch (itemType)
    {
    case ITEM_SIEGE_POTION:
    case ITEM_ANTIDOTE:
    case ITEM_ALE:
    case ITEM_TOWN_PORTAL_SCROLL:
    case ITEM_REMEDY_OF_LOVE:
    case ITEM_JACK_OLANTERN_BLESSINGS:
    case ITEM_JACK_OLANTERN_WRATH:
    case ITEM_JACK_OLANTERN_CRY:
    case ITEM_JACK_OLANTERN_FOOD:
    case ITEM_JACK_OLANTERN_DRINK:
    case ITEM_ELITE_HEALING_POTION:
    case ITEM_ELITE_MANA_POTION:
    case ITEM_ELIXIR_OF_STRENGTH:
    case ITEM_ELIXIR_OF_AGILITY:
    case ITEM_ELIXIR_OF_HEALTH:
    case ITEM_ELIXIR_OF_ENERGY:
    case ITEM_ELIXIR_OF_CONTROL:
    case ITEM_MEDIUM_ELITE_HEALING_POTION:
    case ITEM_CHERRY_BLOSSOM_WINE:
    case ITEM_CHERRY_BLOSSOM_RICE_CAKE:
    case ITEM_CHERRY_BLOSSOM_FLOWER_PETAL:
    case ITEM_ELITE_SD_POTION:
        return itemType != ITEM_REMEDY_OF_LOVE || itemLevel == 0;
    default:
        return false;
    }
}

mu::ui::window::HotKeyItemTypes ItemTypesInUseOrder(Data::Items::ItemTag tag)
{
    return mu::ui::window::HotKeyItemTypes(g_ItemDatabase.GetItemTypesWithTagInUseOrder(tag));
}
} // namespace

mu::ui::window::HotKeyItemTypes mu::ui::window::GetHotKeyItemTypes(int hotKey, int boundType, int boundLevel)
{
    using Data::Items::ItemTag;

    if (IsSingleHotKeyItem(boundType, boundLevel))
    {
        return HotKeyItemTypes(boundType);
    }
    if (GameLogic::Items::IsShieldPotionType(boundType))
    {
        return ItemTypesInUseOrder(ItemTag::ShieldPotion);
    }
    if (GameLogic::Items::IsComplexPotionType(boundType))
    {
        return ItemTypesInUseOrder(ItemTag::ComplexPotion);
    }

    // Q and W take the healing or mana potions, E and R also something else when bound to neither.
    const bool healing = GameLogic::Items::IsHealingPotionType(boundType);
    const bool mana = GameLogic::Items::IsManaPotionType(boundType);
    switch (hotKey)
    {
    case HOTKEY_Q:
        return ItemTypesInUseOrder(mana ? ItemTag::ManaPotion : ItemTag::HealingPotion);
    case HOTKEY_W:
        return ItemTypesInUseOrder(healing ? ItemTag::HealingPotion : ItemTag::ManaPotion);
    case HOTKEY_E:
        if (healing || mana)
        {
            return ItemTypesInUseOrder(healing ? ItemTag::HealingPotion : ItemTag::ManaPotion);
        }
        return HotKeyItemTypes(ITEM_ANTIDOTE);
    case HOTKEY_R:
        if (healing || mana)
        {
            return ItemTypesInUseOrder(healing ? ItemTag::HealingPotion : ItemTag::ManaPotion);
        }
        return ItemTypesInUseOrder(ItemTag::ShieldPotion);
    default:
        return HotKeyItemTypes(std::span<const int>());
    }
}

int mu::ui::window::CItemHotKey::GetHotKeyItemIndex(int iType, bool bItemCount)
{
    int j;

    int iItemCount = 0;
    ITEM* pItem = NULL;

    int iNumberofItems = g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems();
    for (const int i : GetHotKeyItemTypes(iType, m_iHotKeyItemType[iType], m_iHotKeyItemLevel[iType]))
    {
        if (bItemCount)
        {
            for (j = 0; j < iNumberofItems; ++j)
            {
                pItem = g_pMyInventory->GetInventoryCtrl()->GetItem(j);
                if (pItem == NULL)
                {
                    continue;
                }

                if (
                    (pItem->Type == i && pItem->Level == m_iHotKeyItemLevel[iType])
                    || (pItem->Type == i && GameLogic::Items::IsHealingPotion(pItem))
                    )
                {
                    if (pItem->Type == ITEM_ALE
                        || pItem->Type == ITEM_TOWN_PORTAL_SCROLL
                        || pItem->Type == ITEM_REMEDY_OF_LOVE
                        )
                    {
                        iItemCount++;
                    }
                    else
                    {
                        iItemCount += pItem->Durability;
                    }
                }
            }
        }
        else
        {
            int iIndex = -1;
            if (GameLogic::Items::IsHealingPotionType(i))
            {
                iIndex = g_pMyInventory->FindItemReverseIndex(i);
            }
            else
            {
                iIndex = g_pMyInventory->FindItemReverseIndex(i, m_iHotKeyItemLevel[iType]);
            }

            if (-1 != iIndex)
            {
                pItem = g_pMyInventory->FindItem(iIndex);
                if ((pItem->Type != ITEM_SIEGE_POTION
                    && pItem->Type != ITEM_TOWN_PORTAL_SCROLL
                    && pItem->Type != ITEM_REMEDY_OF_LOVE)
                    || pItem->Level == m_iHotKeyItemLevel[iType]
                    )
                {
                    return iIndex;
                }
            }
        }
    }

    if (bItemCount == true)
    {
        return iItemCount;
    }

    return -1;
}

int mu::ui::window::CItemHotKey::GetHotKeyItemCount(int iType)
{
    return 0;
}

void mu::ui::window::CItemHotKey::SetHotKey(int iHotKey, int iItemType, int iItemLevel)
{
    if (iHotKey != -1 && CMyInventory::CanRegisterItemHotKey(iItemType) == true
        )
    {
        m_iHotKeyItemType[iHotKey] = iItemType;
        m_iHotKeyItemLevel[iHotKey] = iItemLevel;
    }
    else
    {
        m_iHotKeyItemType[iHotKey] = -1;
        m_iHotKeyItemLevel[iHotKey] = 0;
    }
}

int mu::ui::window::CItemHotKey::GetHotKey(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemType[iHotKey];
    }

    return -1;
}

int mu::ui::window::CItemHotKey::GetHotKeyLevel(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemLevel[iHotKey];
    }

    return 0;
}

ITEM* mu::ui::window::CItemHotKey::GetSlotItem(int iSlotIndex)
{
    const int iIndex = GetHotKeyItemIndex(iSlotIndex);
    return iIndex != -1 ? g_pMyInventory->FindItem(iIndex) : nullptr;
}

void mu::ui::window::CItemHotKey::SyncSlotIcons(Rml::ElementDocument* document)
{
    if (document == nullptr)
        return;
    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        Rml::Element* icon = document->GetElementById("item_icon_" + std::to_string(i));
        if (icon == nullptr)
            continue;
        const bool filled = GetSlotItem(i) != nullptr;
        icon->SetClass("hidden", !filled);
        m_SlotTargets[i]->Sync(icon, m_bSlotIconsShown && filled);
    }
}

void mu::ui::window::CItemHotKey::SetSlotIconsShown(bool shown)
{
    m_bSlotIconsShown = shown;
    if (shown)
        return;
    for (auto& target : m_SlotTargets)
        target->Disable();
}

void mu::ui::window::CItemHotKey::RenderSlot(int iSlotIndex, const Rml::Vector2f& offset, const Rml::Vector2f& size)
{
    if (ITEM* pItem = GetSlotItem(iSlotIndex))
        RenderItem3DWithHover(offset.x, offset.y, size.x, size.y, pItem->Type, pItem->Level, 0, 0, m_iHoveredSlot == iSlotIndex);
}

void mu::ui::window::CItemHotKey::OnHotkeySlotRightClick(int iSlotIndex)
{
    // RmlUi's Context now does hit-testing for these 4 slots (data-event-mouseup). `Hero->Dead !=
    // 0` reproduces the old call site's own alive guard.
    if (Hero->Dead != 0)
    {
        return;
    }
    int iIndex = GetHotKeyItemIndex(iSlotIndex);
    if (iIndex != -1)
    {
        SendRequestUse(iIndex, 0);
    }
}

int mu::ui::window::CItemHotKey::GetSlotItemCount(int iSlotIndex)
{
    return GetHotKeyItemIndex(iSlotIndex, true);
}
