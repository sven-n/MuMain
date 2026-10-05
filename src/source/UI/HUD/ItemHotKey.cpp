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
        m_SlotTargets[i] = std::make_unique<UI::RmlBridge::RenderTarget>(
            [this, i](std::uint32_t width, std::uint32_t height) { RenderSlot(i, width, height); });
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
        auto& target = *m_SlotTargets[i];
        const bool filled = GetSlotItem(i) != nullptr;
        target.SetEnabled(m_bSlotIconsShown && filled);
        if (filled)
        {
            // The box is in screen pixels: the item is drawn at the size it is shown, never upscaled.
            const auto size = icon->GetBox().GetSize(Rml::BoxArea::Content);
            target.Resize(static_cast<std::uint32_t>(std::lround(size.x)),
                          static_cast<std::uint32_t>(std::lround(size.y)));
        }
        const Rml::String source = filled ? target.Source() : Rml::String();
        if (icon->GetAttribute<Rml::String>("src", "") != source)
            icon->SetAttribute("src", source);
    }
}

void mu::ui::window::CItemHotKey::SetSlotIconsShown(bool shown)
{
    m_bSlotIconsShown = shown;
    if (shown)
        return;
    for (auto& target : m_SlotTargets)
        target->SetEnabled(false);
}

// The item camera C3DCamera::Render() sets up -- an identity view at a 1-degree field of view --
// with its projection cropped to one slot-sized rectangle, so that rectangle fills the target. At
// that field of view where the rectangle sits barely matters, so it is centred, on-axis; only its
// size frames the item, and every per-item offset in RenderItem3D() applies exactly as before.
void mu::ui::window::CItemHotKey::RenderSlot(int iSlotIndex, std::uint32_t width, std::uint32_t height)
{
    ITEM* pItem = GetSlotItem(iSlotIndex);
    if (pItem == nullptr || width == 0 || height == 0)
        return;

    const float windowWidth = static_cast<float>(WindowWidth);
    const float windowHeight = static_cast<float>(WindowHeight);
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    const float x = (windowWidth - w) * 0.5f;
    const float y = (windowHeight - h) * 0.5f;

    // gluPerspective2() and the identity view overwrite g_Camera, which picking reads.
    SaveCameraPerspective();
    // The rectangle is in window pixels, which is what ScreenToWorldRay() turns it into.
    const UI::Scaling::ScopedActiveTransform pixels({1.f, 1.f, 0.f, 0.f, 1.f});

    auto& renderer = mu::GetRenderer();
    renderer.SetMatrixMode(GL_PROJECTION);
    renderer.PushMatrix();
    renderer.LoadIdentity();
    const float scaleX = windowWidth / w;
    const float scaleY = windowHeight / h;
    const float centerX = (2.f * x + w) / windowWidth - 1.f;
    const float centerY = 1.f - (2.f * y + h) / windowHeight;
    renderer.Translate(-centerX * scaleX, -centerY * scaleY, 0.f);
    renderer.Scale(scaleX, scaleY, 1.f);
    // gluPerspective2() takes the camera's screen centre from the viewport; the capture brings its own.
    SetRenderViewport(0, 0, WindowWidth, WindowHeight);
    gluPerspective2(1.f, windowWidth / windowHeight, RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PushMatrix();
    renderer.LoadIdentity();
    CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
    EnableDepthTest();
    EnableDepthMask();

    RenderItem3DWithHover(x, y, w, h, pItem->Type, pItem->Level, 0, 0, m_iHoveredSlot == iSlotIndex);

    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PopMatrix();
    renderer.SetMatrixMode(GL_PROJECTION);
    renderer.PopMatrix();
    RestoreCameraPerspective();
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
