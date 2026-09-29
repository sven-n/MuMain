
#include "stdafx.h"
#include "UI/Inventory/ItemEnduranceInfo.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Scaling/UITransform.h"
#include "I18N/All.h"

#include "Character/CharacterManager.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <string>
#include <utility>

#ifdef PJH_FIX_SPRIT
#include "GameLogic/Pets/GIPetManager.h"
#endif //PJH_FIX_SPRIT



using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The equipped helper that has an HP frame (guardian angel, imp, horns, dark horse, pets).
bool HasHelperLifeFrame()
{
    return Hero->Helper.Type >= MODEL_HELPER && Hero->Helper.Type <= MODEL_DARK_HORSE_ITEM ||
           Hero->Helper.Type == MODEL_DEMON || Hero->Helper.Type == MODEL_SPIRIT_OF_GUARDIAN ||
           Hero->Helper.Type == MODEL_PET_RUDOLF || Hero->Helper.Type == MODEL_PET_PANDA ||
           Hero->Helper.Type == MODEL_PET_UNICORN || Hero->Helper.Type == MODEL_PET_SKELETON ||
           Hero->Helper.Type == MODEL_HORN_OF_FENRIR;
}

// The HP frame's name of the equipped helper (HasHelperLifeFrame()).
void HelperLifeFrameName(wchar_t* szText)
{
    switch (Hero->Helper.Type)
    {
    case MODEL_HELPER:
    {
        mu_swprintf(szText, I18N::Game::GuardianAngel);
    }
    break;
    case MODEL_IMP:
    {
        ITEM_ATTRIBUTE* p = &ItemAttribute[Hero->Helper.Type - MODEL_SWORD];
        mu_swprintf(szText, p->Name);
    }
    break;
    case MODEL_HORN_OF_UNIRIA:
    {
        mu_swprintf(szText, I18N::Game::Uniria);
    }
    break;
    case MODEL_HORN_OF_DINORANT:
    {
        mu_swprintf(szText, I18N::Game::Dinorant);
    }
    break;
    case MODEL_DARK_HORSE_ITEM:
    {
        mu_swprintf(szText, I18N::Game::DarkHorse);
    }
    break;
    case MODEL_HORN_OF_FENRIR:
    {
        mu_swprintf(szText, I18N::Game::Fenrir);
    }
    break;
    case MODEL_DEMON:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_DEMON].Name);
    }
    break;
    case MODEL_SPIRIT_OF_GUARDIAN:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_SPIRIT_OF_GUARDIAN].Name);
    }
    break;
    case MODEL_PET_RUDOLF:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_PET_RUDOLF].Name);
    }
    break;
    case MODEL_PET_PANDA:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_PET_PANDA].Name);
    }
    break;
    case MODEL_PET_UNICORN:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_PET_UNICORN].Name);
    }
    break;
    case MODEL_PET_SKELETON:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_PET_SKELETON].Name);
    }
    break;
    }
}

// The elf's arrow / bolt count line ("Arrows %d (%d)"), false when none is shown.
bool ArrowCountText(int arrowType, int bowArrows, int crossbowBolts, wchar_t* szText)
{
    int iNumEquipedArrowDurability = 0;
    int iNumArrowSetInInven = 0;

    if (arrowType == bowArrows)
    {
        iNumArrowSetInInven = g_pMyInventory->GetNumItemByType(bowArrows);

        if (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type == ITEM_ARROWS)
        {
            iNumEquipedArrowDurability = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Durability;
        }

        if ((iNumArrowSetInInven == 0) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != ITEM_ARROWS))
            return false;

        mu_swprintf(szText, I18N::Game::ArrowsDD, iNumEquipedArrowDurability, iNumArrowSetInInven);
        return true;
    }
    if (arrowType == crossbowBolts)
    {
        iNumArrowSetInInven = g_pMyInventory->GetNumItemByType(crossbowBolts);

        if (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type == ITEM_BOLT)
        {
            iNumEquipedArrowDurability = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability;
        }

        if ((iNumArrowSetInInven == 0) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != ITEM_BOLT))
            return false;

        mu_swprintf(szText, I18N::Game::BoltsDD, iNumEquipedArrowDurability, iNumArrowSetInInven);
        return true;
    }
    return false;
}
} // namespace

CItemEnduranceInfo::CItemEnduranceInfo()
{
    memset(&m_UIStartPos, 0, sizeof(POINT));
    memset(&m_ItemDurUIStartPos, 0, sizeof(POINT));
    m_iTextEndPosX = 0;
    m_iCurArrowType = ARROWTYPE_NONE;
    m_iTooltipIndex = -1;
}

CItemEnduranceInfo::~CItemEnduranceInfo()
{
    Release();
}

bool mu::ui::window::CItemEnduranceInfo::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_ITEM_ENDURANCE_INFO, this);

    SetPos(x, y);
    LoadImages();
    InitImageIndex();
    Show(true);
    return true;
}

void mu::ui::window::CItemEnduranceInfo::Release()
{
    UnloadImages();

    if (m_themeReloadRegistered)
    {
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_themeReloadRegistered = false;
    }
    // Hidden directly: RmlUi renders last in the frame regardless of scene.
    if (m_pRmlDoc != nullptr)
        m_pRmlDoc->Hide();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CItemEnduranceInfo::SetPos(int x, int y)
{
    m_UIStartPos.x = x;
    m_UIStartPos.y = y;

    m_ItemDurUIStartPos.x = GetScreenWidth() - ITEM_DUR_WIDTH - 2;
    m_ItemDurUIStartPos.y = 140;

    m_iTextEndPosX = m_UIStartPos.x + PETHP_FRAME_WIDTH;
}

void mu::ui::window::CItemEnduranceInfo::SetPos(int x)
{
    m_ItemDurUIStartPos.x = x - ITEM_DUR_WIDTH - 2;
    m_ItemDurUIStartPos.y = 140;
}

bool mu::ui::window::CItemEnduranceInfo::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    {
        UI::Scaling::ScopedActiveTransform layout(UI::Scaling::ScreenOverlayTransform(WindowWidth, WindowHeight), true);
        int iNextPosY = m_UIStartPos.y;

        if (Hero->Helper.Type >= MODEL_HELPER && Hero->Helper.Type <= MODEL_DARK_HORSE_ITEM ||
            Hero->Helper.Type == MODEL_DEMON || Hero->Helper.Type == MODEL_SPIRIT_OF_GUARDIAN ||
            Hero->Helper.Type == MODEL_PET_RUDOLF || Hero->Helper.Type == MODEL_PET_PANDA ||
            Hero->Helper.Type == MODEL_PET_UNICORN || Hero->Helper.Type == MODEL_PET_SKELETON ||
            Hero->Helper.Type == MODEL_HORN_OF_FENRIR)
        {
            if (CheckMouseIn(m_UIStartPos.x, iNextPosY, PETHP_FRAME_WIDTH, PETHP_FRAME_HEIGHT))
                return false;

            iNextPosY += (static_cast<int>(UI_INTERVAL_HEIGHT) + PETHP_FRAME_HEIGHT);
        }

        if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
        {
            if (Hero->m_pPet != NULL)
            {
                if (CheckMouseIn(m_UIStartPos.x, iNextPosY, PETHP_FRAME_WIDTH, PETHP_FRAME_HEIGHT))
                    return false;

                iNextPosY += (static_cast<int>(UI_INTERVAL_HEIGHT) + PETHP_FRAME_HEIGHT);
            }
        }

        if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
        {
            if (SummonLife > 0)
            {
                if (CheckMouseIn(m_UIStartPos.x, iNextPosY, PETHP_FRAME_WIDTH, PETHP_FRAME_HEIGHT))
                    return false;

                iNextPosY += (static_cast<int>(UI_INTERVAL_HEIGHT) + PETHP_FRAME_HEIGHT);
            }
        }
    }

    bool bRenderRingWarning = false;
    int icntItemDurIcon = 0;
    auto ItemDurPos = POINT(m_ItemDurUIStartPos);

    for (int i = EQUIPMENT_WEAPON_RIGHT; i < MAX_EQUIPMENT; ++i)
    {
        ITEM* pItem = &CharacterMachine->Equipment[i];

        if ((pItem->bPeriodItem == true) && (pItem->bExpiredPeriod == false))
        {
            continue;
        }

        if (i == EQUIPMENT_HELPER)
        {
            continue;
        }

        if (pItem->Type == -1)
        {
            continue;
        }

        if (i == EQUIPMENT_WEAPON_RIGHT)
        {
            if (pItem->Type == ITEM_ARROWS)
            {
                continue;
            }
        }
        else if (i == EQUIPMENT_WEAPON_LEFT)
        {
            if (pItem->Type == ITEM_BOLT)
            {
                continue;
            }
        }

        int iLevel = pItem->Level;

        if (i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT)
        {
            if (pItem->Type == ITEM_WIZARDS_RING && iLevel == 1
                || iLevel == 2)
            {
                continue;
            }
        }

        ITEM_ATTRIBUTE* pItemAtt = &ItemAttribute[pItem->Type];
        int iMaxDurability = CalcMaxDurability(pItem, pItemAtt, iLevel);

        if (pItem->Durability <= iMaxDurability * 0.5f)
        {
            if (i == EQUIPMENT_RING_RIGHT)
            {
                if (CheckMouseIn(ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH / 2, ITEM_DUR_HEIGHT))
                {
                    m_iTooltipIndex = i;
                    return false;
                }
                bRenderRingWarning = true;
            }
            else if (i == EQUIPMENT_RING_LEFT)
            {
                if (CheckMouseIn(ItemDurPos.x + (ITEM_DUR_WIDTH / 2), ItemDurPos.y, ITEM_DUR_WIDTH / 2, ITEM_DUR_HEIGHT))
                {
                    m_iTooltipIndex = i;
                    return false;
                }
                bRenderRingWarning = false;
            }
            else
            {
                if (CheckMouseIn(ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH, ITEM_DUR_HEIGHT))
                {
                    m_iTooltipIndex = i;
                    return false;
                }
            }

            if (bRenderRingWarning == false)
            {
                icntItemDurIcon++;
                ItemDurPos.y += (static_cast<int>(ITEM_DUR_HEIGHT) + UI_INTERVAL_WIDTH);

                if (icntItemDurIcon % 2 == 0)
                {
                    ItemDurPos.y = m_ItemDurUIStartPos.y;
                    ItemDurPos.x -= (static_cast<int>(ITEM_DUR_WIDTH) + UI_INTERVAL_WIDTH);
                }
            }
        }
    }
    return true;
}

bool mu::ui::window::CItemEnduranceInfo::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CItemEnduranceInfo::Update()
{
    if (!IsVisible())
        return true;

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
    {
        if (gCharacterManager.GetEquipedBowType() == BOWTYPE_BOW)
        {
            m_iCurArrowType = ARROWTYPE_BOW;
        }
        else if (gCharacterManager.GetEquipedBowType() == BOWTYPE_CROSSBOW)
        {
            m_iCurArrowType = ARROWTYPE_CROSSBOW;
        }
        else
        {
            m_iCurArrowType = ARROWTYPE_NONE;
        }
    }

    return true;
}

bool mu::ui::window::CItemEnduranceInfo::Render()
{
    BuildRmlUi();
    if (m_pRmlDoc != nullptr)
    {
        SyncView();
        return true;
    }

    EnableAlphaTest();
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    RenderLeft();
    RenderRight();
    DisableAlphaBlend();
    return true;
}

//---------------------------------------------------------------------------------------------

void mu::ui::window::CItemEnduranceInfo::RenderLeft()
{
    UI::Scaling::ScopedActiveTransform layout(UI::Scaling::ScreenOverlayTransform(WindowWidth, WindowHeight));

    int iNextPosY = m_UIStartPos.y;

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
    {
        if (RenderNumArrow(m_UIStartPos.x, iNextPosY))
        {
            iNextPosY += (UI_INTERVAL_HEIGHT + 10);
        }
    }

    if (RenderEquipedHelperLife(m_UIStartPos.x, iNextPosY))
    {
        iNextPosY += (static_cast<int>(UI_INTERVAL_HEIGHT) + PETHP_FRAME_HEIGHT);
    }

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
    {
        if (RenderEquipedPetLife(m_UIStartPos.x, iNextPosY))
        {
            iNextPosY += (static_cast<int>(UI_INTERVAL_HEIGHT) + PETHP_FRAME_HEIGHT);
        }
    }

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
    {
        if (RenderSummonMonsterLife(m_UIStartPos.x, iNextPosY))
        {
            iNextPosY += (static_cast<int>(UI_INTERVAL_HEIGHT) + PETHP_FRAME_HEIGHT);
        }
    }
}

void mu::ui::window::CItemEnduranceInfo::RenderRight()
{
    RenderItemEndurance(m_ItemDurUIStartPos.x, m_ItemDurUIStartPos.y);
}

bool mu::ui::window::CItemEnduranceInfo::BtnProcess()
{
    return false;
}

float mu::ui::window::CItemEnduranceInfo::GetLayerDepth()
{
    return 3.5f;
}

void mu::ui::window::CItemEnduranceInfo::OpenningProcess()
{
}

void mu::ui::window::CItemEnduranceInfo::ClosingProcess()
{
}

void mu::ui::window::CItemEnduranceInfo::InitImageIndex()
{
    m_iItemDurImageIndex[EQUIPMENT_WEAPON_RIGHT] = IMAGE_ITEM_DUR_WEAPON;
    m_iItemDurImageIndex[EQUIPMENT_WEAPON_LEFT] = IMAGE_ITEM_DUR_SHIELD;
    m_iItemDurImageIndex[EQUIPMENT_HELM] = IMAGE_ITEM_DUR_CAP;
    m_iItemDurImageIndex[EQUIPMENT_ARMOR] = IMAGE_ITEM_DUR_UPPER;
    m_iItemDurImageIndex[EQUIPMENT_PANTS] = IMAGE_ITEM_DUR_LOWER;
    m_iItemDurImageIndex[EQUIPMENT_GLOVES] = IMAGE_ITEM_DUR_GLOVES;
    m_iItemDurImageIndex[EQUIPMENT_BOOTS] = IMAGE_ITEM_DUR_BOOTS;
    m_iItemDurImageIndex[EQUIPMENT_WING] = IMAGE_ITEM_DUR_WING;
    m_iItemDurImageIndex[EQUIPMENT_HELPER] = -1;
    m_iItemDurImageIndex[EQUIPMENT_AMULET] = IMAGE_ITEM_DUR_NECKLACE;
    m_iItemDurImageIndex[EQUIPMENT_RING_RIGHT] = IMAGE_ITEM_DUR_RING;
    m_iItemDurImageIndex[EQUIPMENT_RING_LEFT] = IMAGE_ITEM_DUR_RING;
}

void mu::ui::window::CItemEnduranceInfo::RenderHPUI(int iX, int iY, wchar_t* pszName, int iLife, int iMaxLife/*=255*/, bool bWarning/*=false*/)
{
    EnableAlphaTest();

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    // HPUI_FRAME
    const unsigned int backgroundColor = bWarning ? 0xB3330000u : 0xB3000000u;
    RenderColorQuadARGB(iX + 2, iY + 2, PETHP_FRAME_WIDTH - 4, PETHP_FRAME_HEIGHT - 10, backgroundColor);
    EndRenderColor();

    RenderImage(IMAGE_PETHP_FRAME, iX, iY, PETHP_FRAME_WIDTH, PETHP_FRAME_HEIGHT);

    // HPUI_Bar
    float fLife = ((float)iLife / (float)iMaxLife) * (float)PETHP_BAR_WIDTH;
    RenderImage(IMAGE_PETHP_BAR, iX + 4, iY + PETHP_FRAME_HEIGHT - PETHP_BAR_HEIGHT - 4, fLife, PETHP_BAR_HEIGHT);
    g_pRenderText->RenderText(iX + (PETHP_FRAME_WIDTH / 2), iY + 5, pszName, 0, 0, RT3_WRITE_CENTER);

    DisableAlphaBlend();
}

void mu::ui::window::CItemEnduranceInfo::RenderTooltip(int iX, int iY, const ITEM* pItem, const DWORD& dwTextColor)
{
    ITEM_ATTRIBUTE* pItemAtt = &ItemAttribute[pItem->Type];
    int iLevel = pItem->Level;
    int iMaxDurability = CalcMaxDurability(pItem, pItemAtt, iLevel);

    wchar_t szText[256] = {};
    mu_swprintf(szText, L"%ls (%d/%d)", pItemAtt->Name, pItem->Durability, iMaxDurability);
    g_pRenderText->SetBgColor(0, 0, 0, 128);
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(dwTextColor);
    const int iTooltipWidth = g_pRenderText->MeasureText(szText, static_cast<int>(wcslen(szText))).cx;

    if (iX + (iTooltipWidth / 2) > GetScreenWidth())
    {
        iX = GetScreenWidth() - (iTooltipWidth / 2);
    }

    g_pRenderText->RenderText(iX, iY, szText, 0, 0, RT3_WRITE_CENTER);
}

bool mu::ui::window::CItemEnduranceInfo::RenderEquipedHelperLife(int iX, int iY)
{
    if (!HasHelperLifeFrame())
        return false;

    wchar_t szText[256] = {};
    HelperLifeFrameName(szText);

    int iLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;

    RenderHPUI(iX, iY, szText, iLife);

    return true;
}

bool mu::ui::window::CItemEnduranceInfo::RenderEquipedPetLife(int iX, int iY)
{
    if (Hero->m_pPet == NULL)
        return false;

    wchar_t szText[256] = {};
    mu_swprintf(szText, I18N::Game::DarkRaven);

    int iLife = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability;

    RenderHPUI(iX, iY, szText, iLife);
    return true;
}

bool mu::ui::window::CItemEnduranceInfo::RenderSummonMonsterLife(int iX, int iY)
{
    if (SummonLife <= 0)
        return false;

    wchar_t szText[256] = {};
    mu_swprintf(szText, I18N::Game::SummonedMonsterHP);

    RenderHPUI(iX, iY, szText, SummonLife, 100);

    return true;
}

bool mu::ui::window::CItemEnduranceInfo::RenderNumArrow(int iX, int iY)
{
    wchar_t szText[256] = {};
    if (!ArrowCountText(m_iCurArrowType, ARROWTYPE_BOW, ARROWTYPE_CROSSBOW, szText))
        return false;

    g_pRenderText->SetBgColor(0, 0, 0, 180);
    g_pRenderText->SetTextColor(255, 160, 0, 255);
    g_pRenderText->RenderText(iX, iY, szText, 0, 0, RT3_SORT_LEFT);

    return true;
}

bool mu::ui::window::CItemEnduranceInfo::RenderItemEndurance(int ix, int iY)
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_TRADE))
        return false;

    auto ItemDurPos = POINT(m_ItemDurUIStartPos);
    int icntItemDurIcon = 0;
    int iTempImageIndex;
    bool bRenderRingWarning = false;

    for (int i = EQUIPMENT_WEAPON_RIGHT; i < MAX_EQUIPMENT; ++i)
    {
        ITEM* pItem = &CharacterMachine->Equipment[i];

        iTempImageIndex = m_iItemDurImageIndex[i];

        if ((pItem->bPeriodItem == true) && (pItem->bExpiredPeriod == false))
        {
            continue;
        }

        if (i == EQUIPMENT_HELPER)
        {
            continue;
        }

        if (pItem->Type == -1)
        {
            continue;
        }

        if (i == EQUIPMENT_WEAPON_RIGHT)
        {
            if (pItem->Type == ITEM_ARROWS)
            {
                continue;
            }
        }
        else if (i == EQUIPMENT_WEAPON_LEFT)
        {
            if (gCharacterManager.GetEquipedBowType(pItem) == BOWTYPE_BOW)
            {
                iTempImageIndex = m_iItemDurImageIndex[EQUIPMENT_WEAPON_RIGHT];
            }

            if (pItem->Type == ITEM_BOLT)
            {
                continue;
            }
        }

        int iLevel = pItem->Level;

        if (i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT)
        {
            if (pItem->Type == ITEM_WIZARDS_RING && iLevel == 1
                || iLevel == 2)
            {
                continue;
            }
        }

        ITEM_ATTRIBUTE* pItemAtt = &ItemAttribute[pItem->Type];
        int iMaxDurability = CalcMaxDurability(pItem, pItemAtt, iLevel);

        if (pItem->Durability > iMaxDurability * 0.5f)
        {
            continue;
        }

        EnableAlphaTest();

        if (i != EQUIPMENT_RING_LEFT || bRenderRingWarning != true)
        {
            RenderImage(iTempImageIndex, ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH, ITEM_DUR_HEIGHT);
        }

        if (i == EQUIPMENT_RING_RIGHT)
        {
            bRenderRingWarning = true;
        }

        unsigned int warningColor = 0x80FFFF00u;
        if (pItem->Durability == 0)
            warningColor = 0x80FF0000u;
        else if (pItem->Durability <= iMaxDurability * 0.2f)
            warningColor = 0x80FF3300u;
        else if (pItem->Durability <= iMaxDurability * 0.3f)
            warningColor = 0x80FF8000u;

        if (i == EQUIPMENT_RING_RIGHT)
        {
            RenderColorQuadARGB(ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH / 2, ITEM_DUR_HEIGHT, warningColor);
        }
        else if (i == EQUIPMENT_RING_LEFT)
        {
            RenderColorQuadARGB(ItemDurPos.x + (ITEM_DUR_WIDTH / 2), ItemDurPos.y, ITEM_DUR_WIDTH / 2,
                ITEM_DUR_HEIGHT, warningColor);
            bRenderRingWarning = false;
        }
        else
        {
            RenderColorQuadARGB(ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH, ITEM_DUR_HEIGHT, warningColor);
        }

        EndRenderColor();
        DisableAlphaBlend();

        if (bRenderRingWarning == false)
        {
            icntItemDurIcon++;
            ItemDurPos.y += (static_cast<int>(ITEM_DUR_HEIGHT) + UI_INTERVAL_WIDTH);

            if (icntItemDurIcon % 2 == 0)
            {
                ItemDurPos.y = m_ItemDurUIStartPos.y;
                ItemDurPos.x -= (static_cast<int>(ITEM_DUR_WIDTH) + UI_INTERVAL_WIDTH);
            }
        }
    }

    if (m_iTooltipIndex != -1)
    {
        ITEM* pItem = &CharacterMachine->Equipment[m_iTooltipIndex];
        DWORD dwColor = 0xFFFFFFFF;

        ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
        int iLevel = pItem->Level;
        int iMaxDurability = CalcMaxDurability(pItem, pItemAttr, iLevel);

        if (pItem->Durability <= (iMaxDurability * 0.5f))
        {
            if (pItem->Durability <= 0)
            {
                dwColor = 0xFF0000FF;
            }
            else if (pItem->Durability <= (iMaxDurability * 0.2f))
            {
                dwColor = 0xFF0053FF;
            }
            else if (pItem->Durability <= (iMaxDurability * 0.3f))
            {
                dwColor = 0xFF00A8FF;
            }
            else if (pItem->Durability <= (iMaxDurability * 0.5f))
            {
                dwColor = 0xFF00FFFF;
            }

            RenderTooltip(MouseX, MouseY - 10, pItem, dwColor);
            m_iTooltipIndex = -1;
        }
    }
    return true;
}

void mu::ui::window::CItemEnduranceInfo::LoadImages()
{
    LoadBitmap(L"Interface\\newui_Pet_Back.tga", IMAGE_PETHP_FRAME, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Pet_HpBar.jpg", IMAGE_PETHP_BAR, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_boots.tga", IMAGE_ITEM_DUR_BOOTS, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_cap.tga", IMAGE_ITEM_DUR_CAP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_gloves.tga", IMAGE_ITEM_DUR_GLOVES, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_lower.tga", IMAGE_ITEM_DUR_LOWER, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_necklace.tga", IMAGE_ITEM_DUR_NECKLACE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_ring.tga", IMAGE_ITEM_DUR_RING, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_shield.tga", IMAGE_ITEM_DUR_SHIELD, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_upper.tga", IMAGE_ITEM_DUR_UPPER, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_weapon.tga", IMAGE_ITEM_DUR_WEAPON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_durable_wing.tga", IMAGE_ITEM_DUR_WING, GL_LINEAR);
}

void mu::ui::window::CItemEnduranceInfo::UnloadImages()
{
    DeleteBitmap(IMAGE_PETHP_FRAME);
    DeleteBitmap(IMAGE_PETHP_BAR);
    DeleteBitmap(IMAGE_ITEM_DUR_BOOTS);
    DeleteBitmap(IMAGE_ITEM_DUR_CAP);
    DeleteBitmap(IMAGE_ITEM_DUR_GLOVES);
    DeleteBitmap(IMAGE_ITEM_DUR_LOWER);
    DeleteBitmap(IMAGE_ITEM_DUR_NECKLACE);
    DeleteBitmap(IMAGE_ITEM_DUR_RING);
    DeleteBitmap(IMAGE_ITEM_DUR_SHIELD);
    DeleteBitmap(IMAGE_ITEM_DUR_UPPER);
    DeleteBitmap(IMAGE_ITEM_DUR_WEAPON);
    DeleteBitmap(IMAGE_ITEM_DUR_WING);
}

//---------------------------------------------------------------------------------------------
// RmlUi view (item_endurance.rml)

namespace
{
Rml::Context* ItemEnduranceContext()
{
    return RmlUiRuntime::Instance().GetContext();
}

template <typename T>
void SyncItemEnduranceField(RmlModelBinder<UI::ItemEndurance::ItemEnduranceRmlModel>& binder,
                            T UI::ItemEndurance::ItemEnduranceRmlModel::* field, const char* name, T value)
{
    auto& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}

// The durability band the original's tint and tooltip colours follow.
const char* DurabilityBand(int durability, int maxDurability)
{
    if (durability <= 0)
        return "zero";
    if (durability <= maxDurability * 0.2f)
        return "fifth";
    if (durability <= maxDurability * 0.3f)
        return "third";
    return "half";
}
} // namespace

void mu::ui::window::CItemEnduranceInfo::SyncDocVisibility(bool sceneAllowsShow)
{
    m_sceneAllowsShow = sceneAllowsShow;
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_pRmlDoc, IsVisible() && sceneAllowsShow);
}

void mu::ui::window::CItemEnduranceInfo::BuildRmlUi()
{
    if (m_pRmlDoc != nullptr || !RmlUiRuntime::Instance().IsCreated() || ItemEnduranceContext() == nullptr)
        return;

    using namespace UI::ItemEndurance;
    const bool modelCreated = m_RmlBinder.Create(ItemEnduranceContext(), "item_endurance",
                                                 [](Rml::DataModelConstructor& c, ItemEnduranceRmlModel& model)
                                                 {
                                                     c.Bind("left_x", &model.leftX);
                                                     c.Bind("left_y", &model.leftY);
                                                     c.Bind("left_scale_x", &model.leftScaleX);
                                                     c.Bind("left_scale_y", &model.leftScaleY);
                                                     c.Bind("right_x", &model.rightX);
                                                     c.Bind("right_y", &model.rightY);
                                                     c.Bind("right_scale_x", &model.rightScaleX);
                                                     c.Bind("right_scale_y", &model.rightScaleY);
                                                     c.Bind("text_px", &model.textPx);
                                                     c.Bind("line_height_px", &model.lineHeightPx);
                                                     c.Bind("bold_text_px", &model.boldTextPx);
                                                     c.Bind("bold_line_height_px", &model.boldLineHeightPx);
                                                     c.Bind("arrows", &model.arrows);
                                                     c.Bind("arrows_left", &model.arrowsLeft);
                                                     c.Bind("arrows_top", &model.arrowsTop);

                                                     auto pet = c.RegisterStruct<PetFrameEntry>();
                                                     pet.RegisterMember("top", &PetFrameEntry::top);
                                                     pet.RegisterMember("bar_width", &PetFrameEntry::barWidth);
                                                     pet.RegisterMember("name", &PetFrameEntry::name);
                                                     pet.RegisterMember("name_centre_x", &PetFrameEntry::nameCentreX);
                                                     pet.RegisterMember("name_top", &PetFrameEntry::nameTop);
                                                     c.RegisterArray<std::vector<PetFrameEntry>>();
                                                     c.Bind("pets", &model.pets);

                                                     auto icon = c.RegisterStruct<DurabilityIconEntry>();
                                                     icon.RegisterMember("left", &DurabilityIconEntry::left);
                                                     icon.RegisterMember("top", &DurabilityIconEntry::top);
                                                     icon.RegisterMember("image", &DurabilityIconEntry::image);
                                                     icon.RegisterMember("tint_left", &DurabilityIconEntry::tintLeft);
                                                     icon.RegisterMember("tint_width", &DurabilityIconEntry::tintWidth);
                                                     icon.RegisterMember("band", &DurabilityIconEntry::band);
                                                     c.RegisterArray<std::vector<DurabilityIconEntry>>();
                                                     c.Bind("icons", &model.icons);

                                                     c.Bind("tooltip", &model.tooltip);
                                                     c.Bind("tooltip_band", &model.tooltipBand);
                                                     c.Bind("tooltip_centre_x", &model.tooltipCentreX);
                                                     c.Bind("tooltip_top", &model.tooltipTop);
                                                 });
    if (modelCreated)
        m_pRmlDoc =
            UI::RmlBridge::LoadThemedDocument(ItemEnduranceContext(), "Data/Interface/RmlUi/item_endurance.rml");
    if (m_pRmlDoc != nullptr && !m_themeReloadRegistered)
    {
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
        m_themeReloadRegistered = true;
    }
}

void mu::ui::window::CItemEnduranceInfo::ReloadRmlTheme()
{
    if (m_pRmlDoc == nullptr)
        return;
    Rml::Context* context = ItemEnduranceContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;
    BuildRmlUi();
}

// Render() under the dock-right transform the window manager set, like the original's.
void mu::ui::window::CItemEnduranceInfo::SyncView()
{
    SyncDocVisibility(m_sceneAllowsShow);
    if (!m_pRmlDoc->IsVisible())
        return;

    SyncLeftColumn();
    SyncIcons();
    SyncTooltip();
}

// The original's RenderLeft() under the screen overlay transform: the elf's arrow count line
// (11 high), then the HP frames of the equipped helper, the dark lord's raven and the elf's summon,
// 24 apart from (2, 26). Each frame: the fill, the frame art, the bar (life / max of 49 texels)
// and the name centred 5 below its top.
void mu::ui::window::CItemEnduranceInfo::SyncLeftColumn()
{
    using namespace UI::ItemEndurance;
    const UI::Scaling::Transform overlay = UI::Scaling::ScreenOverlayTransform(WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform layout(overlay);

    // #pets starts at the frames' x (2); the frames' tops are reference px from the screen's top.
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::leftX, "left_x",
                           UI::Scaling::PositionX(overlay, static_cast<float>(m_UIStartPos.x)));
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::leftY, "left_y", overlay.offsetY);
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::leftScaleX, "left_scale_x", overlay.scaleX);
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::leftScaleY, "left_scale_y", overlay.scaleY);

    g_pRenderText->SetFont(g_hFont);
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::textPx, "text_px",
                           UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, overlay));
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::lineHeightPx, "line_height_px",
                           static_cast<float>(g_pRenderText->MeasureText(L"Q", 1).cy) * overlay.scaleY);

    int iNextPosY = m_UIStartPos.y;
    wchar_t szText[256] = {};

    Rml::String arrows;
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF &&
        ArrowCountText(m_iCurArrowType, ARROWTYPE_BOW, ARROWTYPE_CROSSBOW, szText))
    {
        arrows = StringUtils::WideToNarrow(szText);
        SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::arrowsLeft, "arrows_left",
                               UI::Scaling::PositionX(overlay, static_cast<float>(m_UIStartPos.x)));
        SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::arrowsTop, "arrows_top",
                               UI::Scaling::PositionY(overlay, static_cast<float>(iNextPosY)));
        iNextPosY += (UI_INTERVAL_HEIGHT + 10);
    }
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::arrows, "arrows", std::move(arrows));

    std::vector<PetFrameEntry> pets;
    const auto addFrame = [&](const wchar_t* name, int life, int maxLife)
    {
        PetFrameEntry frame;
        frame.top = static_cast<float>(iNextPosY);
        frame.barWidth = (static_cast<float>(life) / static_cast<float>(maxLife)) * static_cast<float>(PETHP_BAR_WIDTH);
        frame.name = StringUtils::WideToNarrow(name);
        frame.nameCentreX =
            UI::Scaling::PositionX(overlay, static_cast<float>(m_UIStartPos.x + (PETHP_FRAME_WIDTH / 2)));
        frame.nameTop = UI::Scaling::PositionY(overlay, static_cast<float>(iNextPosY + 5));
        pets.push_back(std::move(frame));
        iNextPosY += (static_cast<int>(UI_INTERVAL_HEIGHT) + PETHP_FRAME_HEIGHT);
    };

    if (HasHelperLifeFrame())
    {
        szText[0] = L'\0';
        HelperLifeFrameName(szText);
        addFrame(szText, CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability, 255);
    }
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD && Hero->m_pPet != nullptr)
    {
        addFrame(I18N::Game::DarkRaven, CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability, 255);
    }
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF && SummonLife > 0)
    {
        addFrame(I18N::Game::SummonedMonsterHP, SummonLife, 100);
    }

    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::pets, "pets", std::move(pets));
}

// The original's RenderItemEndurance() under the dock-right transform: every worn item at most half
// durable, in slot order, two per column from (W - 25, 140) leftwards, 25 apart; the rings share one
// icon, each tinting its half (right ring the left half). None while the trade window is open.
void mu::ui::window::CItemEnduranceInfo::SyncIcons()
{
    using namespace UI::ItemEndurance;
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::rightX, "right_x", transform.offsetX);
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::rightY, "right_y", transform.offsetY);
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::rightScaleX, "right_scale_x", transform.scaleX);
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::rightScaleY, "right_scale_y", transform.scaleY);

    std::vector<DurabilityIconEntry> icons;
    if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_TRADE))
    {
        auto ItemDurPos = POINT(m_ItemDurUIStartPos);
        int icntItemDurIcon = 0;
        bool bRenderRingWarning = false;

        for (int i = EQUIPMENT_WEAPON_RIGHT; i < MAX_EQUIPMENT; ++i)
        {
            ITEM* pItem = &CharacterMachine->Equipment[i];
            int iImageIndex = m_iItemDurImageIndex[i];

            if ((pItem->bPeriodItem == true) && (pItem->bExpiredPeriod == false))
                continue;
            if (i == EQUIPMENT_HELPER || pItem->Type == -1)
                continue;
            if (i == EQUIPMENT_WEAPON_RIGHT && pItem->Type == ITEM_ARROWS)
                continue;
            if (i == EQUIPMENT_WEAPON_LEFT)
            {
                if (gCharacterManager.GetEquipedBowType(pItem) == BOWTYPE_BOW)
                    iImageIndex = m_iItemDurImageIndex[EQUIPMENT_WEAPON_RIGHT];
                if (pItem->Type == ITEM_BOLT)
                    continue;
            }

            int iLevel = pItem->Level;
            if ((i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT) &&
                (pItem->Type == ITEM_WIZARDS_RING && iLevel == 1 || iLevel == 2))
                continue;

            ITEM_ATTRIBUTE* pItemAtt = &ItemAttribute[pItem->Type];
            int iMaxDurability = CalcMaxDurability(pItem, pItemAtt, iLevel);
            if (pItem->Durability > iMaxDurability * 0.5f)
                continue;

            DurabilityIconEntry icon;
            icon.left = static_cast<float>(ItemDurPos.x);
            icon.top = static_cast<float>(ItemDurPos.y);
            icon.band = DurabilityBand(pItem->Durability, iMaxDurability);
            if (i != EQUIPMENT_RING_LEFT || bRenderRingWarning != true)
            {
                switch (iImageIndex)
                {
                case IMAGE_ITEM_DUR_BOOTS:
                    icon.image = "boots";
                    break;
                case IMAGE_ITEM_DUR_CAP:
                    icon.image = "cap";
                    break;
                case IMAGE_ITEM_DUR_GLOVES:
                    icon.image = "gloves";
                    break;
                case IMAGE_ITEM_DUR_LOWER:
                    icon.image = "lower";
                    break;
                case IMAGE_ITEM_DUR_NECKLACE:
                    icon.image = "necklace";
                    break;
                case IMAGE_ITEM_DUR_RING:
                    icon.image = "ring";
                    break;
                case IMAGE_ITEM_DUR_SHIELD:
                    icon.image = "shield";
                    break;
                case IMAGE_ITEM_DUR_UPPER:
                    icon.image = "upper";
                    break;
                case IMAGE_ITEM_DUR_WEAPON:
                    icon.image = "weapon";
                    break;
                case IMAGE_ITEM_DUR_WING:
                    icon.image = "wing";
                    break;
                default:
                    break;
                }
            }

            if (i == EQUIPMENT_RING_RIGHT)
            {
                bRenderRingWarning = true;
                icon.tintWidth = static_cast<float>(ITEM_DUR_WIDTH / 2);
            }
            else if (i == EQUIPMENT_RING_LEFT)
            {
                icon.tintLeft = static_cast<float>(ITEM_DUR_WIDTH / 2);
                icon.tintWidth = static_cast<float>(ITEM_DUR_WIDTH / 2);
                bRenderRingWarning = false;
            }
            else
            {
                icon.tintWidth = static_cast<float>(ITEM_DUR_WIDTH);
            }
            icons.push_back(std::move(icon));

            if (bRenderRingWarning == false)
            {
                icntItemDurIcon++;
                ItemDurPos.y += (static_cast<int>(ITEM_DUR_HEIGHT) + UI_INTERVAL_WIDTH);

                if (icntItemDurIcon % 2 == 0)
                {
                    ItemDurPos.y = m_ItemDurUIStartPos.y;
                    ItemDurPos.x -= (static_cast<int>(ITEM_DUR_WIDTH) + UI_INTERVAL_WIDTH);
                }
            }
        }
    }
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::icons, "icons", std::move(icons));
}

// The original's tooltip of the icon UpdateMouseEvent() found under the pointer: "name (dur/max)",
// bold, centred 10 above the pointer and kept inside the screen's right edge, coloured by band.
void mu::ui::window::CItemEnduranceInfo::SyncTooltip()
{
    using namespace UI::ItemEndurance;
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();

    g_pRenderText->SetFont(g_hFontBold);
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::boldTextPx, "bold_text_px",
                           UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform));
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::boldLineHeightPx, "bold_line_height_px",
                           static_cast<float>(g_pRenderText->MeasureText(L"Q", 1).cy) * transform.scaleY);

    Rml::String tooltip;
    Rml::String band;
    if (m_iTooltipIndex != -1 && !g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_TRADE))
    {
        const ITEM* pItem = &CharacterMachine->Equipment[m_iTooltipIndex];
        ITEM_ATTRIBUTE* pItemAtt = &ItemAttribute[pItem->Type];
        const int iMaxDurability = CalcMaxDurability(pItem, pItemAtt, pItem->Level);
        if (pItem->Durability <= (iMaxDurability * 0.5f))
        {
            wchar_t szText[256] = {};
            mu_swprintf(szText, L"%ls (%d/%d)", pItemAtt->Name, pItem->Durability, iMaxDurability);
            const int iTooltipWidth = g_pRenderText->MeasureText(szText, static_cast<int>(wcslen(szText))).cx;
            int iX = MouseX;
            if (iX + (iTooltipWidth / 2) > GetScreenWidth())
                iX = GetScreenWidth() - (iTooltipWidth / 2);

            tooltip = StringUtils::WideToNarrow(szText);
            band = DurabilityBand(pItem->Durability, iMaxDurability);
            SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::tooltipCentreX, "tooltip_centre_x",
                                   UI::Scaling::PositionX(transform, static_cast<float>(iX)));
            SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::tooltipTop, "tooltip_top",
                                   UI::Scaling::PositionY(transform, static_cast<float>(MouseY - 10)));
            m_iTooltipIndex = -1;
        }
    }
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::tooltip, "tooltip", std::move(tooltip));
    SyncItemEnduranceField(m_RmlBinder, &ItemEnduranceRmlModel::tooltipBand, "tooltip_band", std::move(band));
}
