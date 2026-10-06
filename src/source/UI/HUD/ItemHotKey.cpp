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
        if ((pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82))
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

int mu::ui::window::CItemHotKey::GetHotKeyItemIndex(int iType, bool bItemCount)
{
    int iStartItemType = 0, iEndItemType = 0;
    int i, j;

    switch (iType)
    {
    case HOTKEY_Q:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
        }
        break;
    case HOTKEY_W:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
        }
        break;
    case HOTKEY_E:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_ANTIDOTE; iEndItemType = ITEM_ANTIDOTE;
            }
        }
        break;
    case HOTKEY_R:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_SHIELD_POTION; iEndItemType = ITEM_SMALL_SHIELD_POTION;
            }
        }
        break;
    }

    int iItemCount = 0;
    ITEM* pItem = NULL;

    int iNumberofItems = g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems();
    for (i = iStartItemType; i >= iEndItemType; --i)
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
                    || (pItem->Type == i && (pItem->Type >= ITEM_APPLE && pItem->Type <= ITEM_LARGE_HEALING_POTION))
                    )
                {
                    if (pItem->Type == ITEM_ALE
                        || pItem->Type == ITEM_TOWN_PORTAL_SCROLL
                        || pItem->Type == ITEM_POTION + 20
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
            if (i >= ITEM_APPLE && i <= ITEM_LARGE_HEALING_POTION)
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
                    && pItem->Type != ITEM_POTION + 20)
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

bool mu::ui::window::CItemHotKey::GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd)
{
    switch (m_iHotKeyItemType[iHotKey])
    {
    case ITEM_SIEGE_POTION:
    case ITEM_ANTIDOTE:
    case ITEM_ALE:
    case ITEM_TOWN_PORTAL_SCROLL:
    case ITEM_POTION + 20:
    case ITEM_JACK_OLANTERN_BLESSINGS:
    case ITEM_JACK_OLANTERN_WRATH:
    case ITEM_JACK_OLANTERN_CRY:
    case ITEM_JACK_OLANTERN_FOOD:
    case ITEM_JACK_OLANTERN_DRINK:
    case ITEM_POTION + 70:
    case ITEM_POTION + 71:
    case ITEM_POTION + 78:
    case ITEM_POTION + 79:
    case ITEM_POTION + 80:
    case ITEM_POTION + 81:
    case ITEM_POTION + 82:
    case ITEM_POTION + 94:
    case ITEM_CHERRY_BLOSSOM_WINE:
    case ITEM_CHERRY_BLOSSOM_RICE_CAKE:
    case ITEM_CHERRY_BLOSSOM_FLOWER_PETAL:
    case ITEM_POTION + 133:
        if (m_iHotKeyItemType[iHotKey] != ITEM_POTION + 20 || m_iHotKeyItemLevel[iHotKey] == 0)
        {
            iStart = iEnd = m_iHotKeyItemType[iHotKey];
            return true;
        }
        break;
    default:
        if (m_iHotKeyItemType[iHotKey] >= ITEM_SMALL_SHIELD_POTION && m_iHotKeyItemType[iHotKey] <= ITEM_LARGE_SHIELD_POTION)
        {
            iStart = ITEM_LARGE_SHIELD_POTION; iEnd = ITEM_SMALL_SHIELD_POTION;
            return true;
        }
        else if (m_iHotKeyItemType[iHotKey] >= ITEM_SMALL_COMPLEX_POTION && m_iHotKeyItemType[iHotKey] <= ITEM_LARGE_COMPLEX_POTION)
        {
            iStart = ITEM_LARGE_COMPLEX_POTION; iEnd = ITEM_SMALL_COMPLEX_POTION;
            return true;
        }
        break;
    }
    return false;
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
