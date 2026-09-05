//////////////////////////////////////////////////////////////////////
// NewUIMoveCommandWindow.cpp: implementation of the CNewUIMoveCommandWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/HUD/NewUIMoveCommandWindow.h"
#include "UI/Core/NewUISystem.h"
#include "Engine/Object/ZzzInterface.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "Core/Utilities/KeyGenerator.h"
#include "Network/Server/ServerListManager.h"
#include "Engine/Object/ZzzOpenData.h"
#include "World/MapInfra/MapManager.h"
#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

using namespace SEASON3B;

namespace
{
    constexpr int MapNameCount = 6;
    constexpr int kDefaultRowHeight = 14;
    constexpr int kStrifeCenterOffsetX = 20;
    constexpr int kMapCenterOffsetX = 62;
    constexpr int kRequiredLevelCenterOffsetX = 119;
    constexpr int kZenCenterOffsetX = 159;
    constexpr int kScrollBarOffsetX = 14;
    constexpr int kScrollThumbOffsetX = 4;

    const std::wstring MapName[MapNameCount] =
    {
        L"Lorencia",
        L"Noria",
        L"Elbeland",
        L"Dungeon",
        L"Devias",
        L"LostTower",
    };

    bool IsLuckySeal(const std::wstring& name)
    {
        if (name.size() != 0) {
            for (int i = 0; i < MapNameCount; ++i) {
                if (name == MapName[i])
                {
                    return true;
                }
            }
        }
        return false;
    }
};

CNewUIMoveCommandWindow::CNewUIMoveCommandWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;

    memset(&m_StartUISubjectName, 0, sizeof(POINT));
    memset(&m_StartMapNamePos, 0, sizeof(POINT));
    memset(&m_MapNameUISize, 0, sizeof(POINT));
    memset(&m_StrifePos, 0, sizeof(POINT));
    memset(&m_MapNamePos, 0, sizeof(POINT));
    memset(&m_ReqLevelPos, 0, sizeof(POINT));
    memset(&m_ReqZenPos, 0, sizeof(POINT));
    m_iSelectedMapName = -1;

    m_iSelectedTextIndex = -1;
    m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
}

CNewUIMoveCommandWindow::~CNewUIMoveCommandWindow()
{
    Release();
}

bool SEASON3B::CNewUIMoveCommandWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MOVEMAP, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void SEASON3B::CNewUIMoveCommandWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIMoveCommandWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    RefreshDataAndLayout();
}

void SEASON3B::CNewUIMoveCommandWindow::RefreshDataAndLayout()
{
    m_listMoveInfoData = CMoveCommandData::GetInstance()->GetMoveCommandDatalist();
    g_pRenderText->SetFont(g_hFont);
    const int measuredFontHeight = g_pRenderText->MeasureText(L"Q", 1).cy;
    m_iRealFontHeight = measuredFontHeight > 0 ? measuredFontHeight + 2 : kDefaultRowHeight;
    m_layout = UI::MoveCommand::CalculateLayout(m_Pos.y, m_iRealFontHeight);

    m_MapNameUISize.x = m_layout.windowWidth;
    m_MapNameUISize.y = m_layout.windowHeight;
    m_StartUISubjectName.x = m_Pos.x + m_layout.windowWidth / 2;
    m_StartUISubjectName.y = m_Pos.y + 4;
    m_StartMapNamePos.x = m_Pos.x + 2;
    m_StartMapNamePos.y = m_layout.listTop;
    m_StrifePos.x = m_Pos.x + kStrifeCenterOffsetX;
    m_StrifePos.y = m_layout.listTop;
    m_MapNamePos.x = m_Pos.x + kMapCenterOffsetX;
    m_MapNamePos.y = m_layout.listTop;
    m_ReqLevelPos.x = m_Pos.x + kRequiredLevelCenterOffsetX;
    m_ReqLevelPos.y = m_layout.listTop;
    m_ReqZenPos.x = m_Pos.x + kZenCenterOffsetX;
    m_ReqZenPos.y = m_layout.listTop;
    SetScrollOffset(m_scrollOffset);
}

void SEASON3B::CNewUIMoveCommandWindow::SetScrollOffset(int offset)
{
    m_scrollOffset = UI::MoveCommand::ClampScrollOffset(offset, m_listMoveInfoData.size(), m_layout.visibleRows);
}

int SEASON3B::CNewUIMoveCommandWindow::VisibleEndIndex() const
{
    return std::min(m_scrollOffset + m_layout.visibleRows, static_cast<int>(m_listMoveInfoData.size()));
}

bool SEASON3B::CNewUIMoveCommandWindow::IsLuckySealBuff()
{
    if (g_isCharacterBuff((&Hero->Object), eBuff_Seal1)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal2)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal3)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal4)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal_HpRecovery)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal_MpRecovery)
        || g_isCharacterBuff((&Hero->Object), eBuff_AscensionSealMaster)
        || g_isCharacterBuff((&Hero->Object), eBuff_WealthSealMaster)
        || g_isCharacterBuff((&Hero->Object), eBuff_NewWealthSeal)
        || g_isCharacterBuff((&Hero->Object), eBuff_PartyExpBonus)
        )
    {
        return true;
    }
    return false;
}

bool SEASON3B::CNewUIMoveCommandWindow::IsMapMove(const std::wstring& src)
{
    if (Hero->Object.Kind == KIND_PLAYER
        && Hero->Object.Type == MODEL_PLAYER
        && Hero->Object.SubType == MODEL_GM_CHARACTER)
    {
        return true;
    }

    if (g_isCharacterBuff((&Hero->Object), eBuff_GMEffect))
    {
        return true;
    }

    if (IsLuckySealBuff() == false) {
        wchar_t lpszStr1[1024]; wchar_t* lpszStr2 = NULL;
        if (src.find(I18N::Game::Warp) != std::wstring::npos) {
            std::wstring temp = I18N::Game::Warp;
            temp += ' ';
            mu_swprintf(lpszStr1, src.c_str());
            wchar_t* context = nullptr;
            lpszStr2 = wcstok_s(lpszStr1, temp.c_str(), &context);
            if (lpszStr2 == NULL) return false;

            SettingCanMoveMap();
            for (const auto* moveInfo : m_listMoveInfoData) {
                if (!wcscmp(lpszStr2, moveInfo->_ReqInfo.szMainMapName)) {
                    if (moveInfo->_bCanMove == true) {
                        return IsLuckySeal(moveInfo->_ReqInfo.szSubMapName);
                    }
                }
            }
            return false;
        }
        else if (src.find(L"/move") != std::wstring::npos) {
            std::wstring temp = L"/move";
            temp += ' ';
            mu_swprintf(lpszStr1, src.c_str());
            wchar_t* context = nullptr;
            lpszStr2 = wcstok_s(lpszStr1, temp.c_str(), &context);
            if (lpszStr2 == NULL) return false;

            SettingCanMoveMap();
            for (const auto* moveInfo : m_listMoveInfoData) {
                if (!wcsicmp(lpszStr2, moveInfo->_ReqInfo.szMainMapName)) {
                    if (moveInfo->_bCanMove == true) {
                        return IsLuckySeal(moveInfo->_ReqInfo.szSubMapName);
                    }
                }
            }
            return false;
        }
        else {
            return IsLuckySeal(src);
        }
    }
    return true;
}

void SEASON3B::CNewUIMoveCommandWindow::SetMoveCommandKey(DWORD dwKey)
{
    m_dwMoveCommandKey = dwKey;
}

DWORD SEASON3B::CNewUIMoveCommandWindow::GetMoveCommandKey()
{
    m_dwMoveCommandKey = g_KeyGenerator.GenerateKeyValue(m_dwMoveCommandKey);

    return m_dwMoveCommandKey;
}

void SEASON3B::CNewUIMoveCommandWindow::SetStrifeMap()
{
    std::list<CMoveCommandData::MOVEINFODATA*>::iterator li;

    if (!g_ServerListManager->IsNonPvP())
    {
        int anStrifeIndex[1] = { 42 };
        int i;
        for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
        {
            (*li)->_bStrife = false;
            for (i = 0; i < 1; ++i)
            {
                if ((*li)->_ReqInfo.index == anStrifeIndex[i])
                {
                    (*li)->_bStrife = true;
                    break;
                }
            }
        }
    }
    else
    {
        for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
            (*li)->_bStrife = false;
    }
}

void SEASON3B::CNewUIMoveCommandWindow::SettingCanMoveMap()
{
    int a = gMapManager.WorldActive;

    DWORD iZen;
    int iLevel, iReqLevel, iReqZen;

    for (auto* moveInfo : m_listMoveInfoData)
    {
        moveInfo->_bCanMove = false;
        moveInfo->_bSelected = false;

        iLevel = CharacterAttribute->Level;
        iZen = CharacterMachine->Gold;
        iReqLevel = moveInfo->_ReqInfo.iReqLevel;
        iReqZen = moveInfo->_ReqInfo.iReqZen;

        if ((gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_DARK || gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_DARK_LORD
            || gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_RAGEFIGHTER)
            && (iReqLevel != 400))
        {
            iReqLevel = int(float(iReqLevel) * 2.f / 3.f);
        }

        if (iLevel >= iReqLevel && (int)iZen >= iReqZen && (int)Hero->PK < PVP_MURDERER1)
        {
            ITEM* pEquipedRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
            ITEM* pEquipedLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
            ITEM* pEquipedHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
            ITEM* pEquipedWing = &CharacterMachine->Equipment[EQUIPMENT_WING];

            if (wcscmp(moveInfo->_ReqInfo.szMainMapName, I18N::Game::Icarus) == 0)
            {
                if (
                    (
                        pEquipedHelper->Type == ITEM_HORN_OF_FENRIR
                        || pEquipedHelper->Type == ITEM_HORN_OF_DINORANT
                        || pEquipedHelper->Type == ITEM_DARK_HORSE_ITEM
                        || pEquipedWing->Type == ITEM_CAPE_OF_LORD
                        || (pEquipedWing->Type >= ITEM_WING_OF_STORM && pEquipedWing->Type <= ITEM_WING_OF_DIMENSION)
                        || (pEquipedWing->Type >= ITEM_WING && pEquipedWing->Type <= ITEM_WINGS_OF_DARKNESS)
                        || (ITEM_WING + 130 <= pEquipedWing->Type && pEquipedWing->Type <= ITEM_WING + 134)
                        || (pEquipedWing->Type >= ITEM_CAPE_OF_FIGHTER && pEquipedWing->Type <= ITEM_CAPE_OF_OVERRULE)
                        || (pEquipedWing->Type == ITEM_WING + 135))
                    && !(pEquipedHelper->Type == ITEM_HORN_OF_UNIRIA)
                    && (g_ChangeRingMgr->CheckBanMoveIcarusMap(pEquipedRightRing->Type, pEquipedLeftRing->Type) == false)
                    )
                {
                    moveInfo->_bCanMove = true;
                }
                else
                {
                    moveInfo->_bCanMove = false;
                }
            }
            else if (wcsncmp(moveInfo->_ReqInfo.szMainMapName, I18N::Game::Atlans, wcslen(I18N::Game::Atlans)) == 0)
            {
                if (pEquipedHelper->Type == ITEM_HORN_OF_UNIRIA || pEquipedHelper->Type == ITEM_HORN_OF_DINORANT)
                {
                    moveInfo->_bCanMove = false;
                }
                else
                {
                    moveInfo->_bCanMove = true;
                }
            }
            else if ((g_ServerListManager->IsNonPvP() == true) && (wcscmp(moveInfo->_ReqInfo.szMainMapName, I18N::Game::Vulcanus) == 0))
            {
                moveInfo->_bCanMove = false;
            }
            else
            {
                moveInfo->_bCanMove = true;
            }
        }

        if (moveInfo->_bCanMove && moveInfo->_bStrife && 0 == Hero->m_byGensInfluence)
            moveInfo->_bCanMove = false;
    }
}

bool SEASON3B::CNewUIMoveCommandWindow::BtnProcess()
{
    const int maximumOffset = UI::MoveCommand::MaximumScrollOffset(m_listMoveInfoData.size(), m_layout.visibleRows);
    const int scrollBarX = m_Pos.x + m_layout.windowWidth - kScrollBarOffsetX;
    const int thumbX = scrollBarX - kScrollThumbOffsetX;
    const int thumbY = UI::MoveCommand::ThumbYForScrollOffset(m_scrollOffset, m_layout, m_listMoveInfoData.size());
    const bool wasDragging = m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED;

    if (!wasDragging
        && maximumOffset > 0
        && CheckMouseIn(thumbX, thumbY, MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT)
        && IsPress(VK_LBUTTON))
    {
        m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_CLICKED;
        m_scrollDragGrabOffsetY = MouseY - thumbY;
    }

    if (wasDragging)
    {
        const auto dragState = UI::MoveCommand::UpdateDragState(
            true, IsRelease(VK_LBUTTON), MouseY, m_scrollDragGrabOffsetY, m_scrollOffset, m_layout,
            m_listMoveInfoData.size());
        SetScrollOffset(dragState.scrollOffset);
        m_iScrollBtnMouseEvent = dragState.dragging
            ? MOVECOMMAND_MOUSEBTN_CLICKED
            : MOVECOMMAND_MOUSEBTN_NORMAL;

        if (dragState.releaseConsumed)
        {
            return true;
        }
    }
    else if (IsRelease(VK_LBUTTON))
    {
        m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y) && IsPress(VK_LBUTTON))
    {
        SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
    }

    SettingCanMoveMap();

    if (CheckMouseIn(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y))
    {
        if (MouseWheel != 0)
            SetScrollOffset(m_scrollOffset - MouseWheel);
        MouseWheel = 0;

        auto li = m_listMoveInfoData.begin();
        for (int i = 0; i < m_scrollOffset && li != m_listMoveInfoData.end(); ++i)
            ++li;

        int visibleIndex = 0;
        while (li != m_listMoveInfoData.end() && m_scrollOffset + visibleIndex < VisibleEndIndex())
        {
            const int itemY = m_layout.listTop + m_iRealFontHeight * visibleIndex;
            if (CheckMouseIn(m_StartMapNamePos.x, itemY, m_layout.windowWidth - 22, m_iRealFontHeight))
            {
                if ((*li)->_bCanMove == true)
                {
                    (*li)->_bSelected = true;

                    if (SEASON3B::IsRelease(VK_LBUTTON))
                    {
                        if (IsTheMapInDifferentServer(gMapManager.WorldActive, (*li)->_ReqInfo.index))
                        {
                            SaveOptions();
                        }

                        SocketClient->ToGameServer()->SendWarpCommandRequest(g_pMoveCommandWindow->GetMoveCommandKey(), (*li)->_ReqInfo.index);

                        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MOVEMAP);
                        return true;
                    }
                }
            }

            ++li;
            ++visibleIndex;
        }

        if (SEASON3B::IsRelease(VK_LBUTTON)
            && CheckMouseIn(m_Pos.x + m_layout.closeLeft, m_layout.closeTop, m_layout.closeWidth, m_iRealFontHeight))
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MOVEMAP);
            return true;
        }
    }
    return false;
}

bool SEASON3B::CNewUIMoveCommandWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED)
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y))
        return false;

    return true;
}

bool SEASON3B::CNewUIMoveCommandWindow::UpdateKeyEvent()
{
    if (IsVisible())
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            Show(false);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool SEASON3B::CNewUIMoveCommandWindow::Update()
{
    return true;
}

void SEASON3B::CNewUIMoveCommandWindow::RenderFrame()
{
    EnableAlphaTest();

    RenderColorQuadARGB((float)m_Pos.x, (float)m_Pos.y, (float)m_layout.windowWidth,
        (float)m_layout.windowHeight, 0xCC000000u);
    RenderColorQuadARGB(m_Pos.x + m_layout.closeLeft, m_layout.closeTop,
        m_layout.closeWidth, m_iRealFontHeight, 0xFF990000u);

    const int scrollBarX = m_Pos.x + m_layout.windowWidth - kScrollBarOffsetX;
    RenderImage(IMAGE_MOVECOMMAND_SCROLL_TOP, scrollBarX, m_layout.scrollTrackTop,
        MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT);

    const int middleTop = m_layout.scrollTrackTop + MOVECOMMAND_SCROLLBAR_TOP_HEIGHT;
    const int middleBottom = m_layout.scrollTrackTop + m_layout.scrollTrackHeight - MOVECOMMAND_SCROLLBAR_TOP_HEIGHT;
    for (int middleY = middleTop; middleY < middleBottom; middleY += MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT)
    {
        const int middleHeight = std::min(static_cast<int>(MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT), middleBottom - middleY);
        RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, scrollBarX, middleY,
            MOVECOMMAND_SCROLLBAR_MIDDLE_WIDTH, middleHeight);
    }

    RenderImage(IMAGE_MOVECOMMAND_SCROLL_BOTTOM, scrollBarX,
        m_layout.scrollTrackTop + m_layout.scrollTrackHeight - MOVECOMMAND_SCROLLBAR_TOP_HEIGHT,
        MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT);

    const int thumbX = scrollBarX - kScrollThumbOffsetX;
    const int thumbY = UI::MoveCommand::ThumbYForScrollOffset(m_scrollOffset, m_layout, m_listMoveInfoData.size());
    if (UI::MoveCommand::MaximumScrollOffset(m_listMoveInfoData.size(), m_layout.visibleRows) > 0)
    {
        const DWORD color = m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED
            ? 0xFFB3B3B3u
            : 0xFFFFFFFFu;
        RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_ON, thumbX, thumbY,
            MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT, 0.f, 0.f, color);
    }
    else
    {
        RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_OFF, thumbX, thumbY,
            MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT);
    }

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(255, 204, 26, 255);
    g_pRenderText->RenderText(m_StartUISubjectName.x, m_StartUISubjectName.y, I18N::Game::WarpCommandWindow, 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(127, 178, 255, 255);
    g_pRenderText->RenderText(m_StrifePos.x, m_StartUISubjectName.y + 20, I18N::Game::BattleZone, 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->RenderText(m_MapNamePos.x, m_StartUISubjectName.y + 20, I18N::Game::Map, 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->RenderText(m_ReqLevelPos.x, m_StartUISubjectName.y + 20, I18N::Game::MinLevel, 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->RenderText(m_ReqZenPos.x, m_StartUISubjectName.y + 20, I18N::Game::Cost, 0, 0, RT3_WRITE_CENTER);
}

bool SEASON3B::CNewUIMoveCommandWindow::Render()
{
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    RenderFrame();

    auto li = m_listMoveInfoData.begin();
    for (int i = 0; i < m_scrollOffset && li != m_listMoveInfoData.end(); ++i)
        ++li;

    int iLevel = CharacterAttribute->Level;
    DWORD iZen = CharacterMachine->Gold;
    int iReqLevel;
    wchar_t szText[24];

    int visibleIndex = 0;
    while (li != m_listMoveInfoData.end() && m_scrollOffset + visibleIndex < VisibleEndIndex())
    {
        const int itemY = m_layout.listTop + m_iRealFontHeight * visibleIndex;

        iReqLevel = (*li)->_ReqInfo.iReqLevel;
        if ((gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_DARK || gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_DARK_LORD
            || gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_RAGEFIGHTER)
            && (iReqLevel != 400))
        {
            iReqLevel = int(float(iReqLevel) * 2.f / 3.f);
        }

        if ((*li)->_bCanMove == true)
        {
            g_pRenderText->SetTextColor(255, 255, 255, 255);

            if ((*li)->_bStrife)
                g_pRenderText->RenderText(m_StrifePos.x, itemY, I18N::Game::Battle2987, 0, 0, RT3_WRITE_CENTER);
            g_pRenderText->RenderText(m_MapNamePos.x, itemY, (*li)->_ReqInfo.szMainMapName, 0, 0, RT3_WRITE_CENTER);
            _itow(iReqLevel, szText, 10);
            g_pRenderText->RenderText(m_ReqLevelPos.x, itemY, szText, 0, 0, RT3_WRITE_CENTER);
            _itow((*li)->_ReqInfo.iReqZen, szText, 10);
            g_pRenderText->RenderText(m_ReqZenPos.x, itemY, szText, 0, 0, RT3_WRITE_CENTER);

            if ((*li)->_bSelected == true)
            {
                RenderColorQuadARGB(m_StartMapNamePos.x, itemY - 1, m_layout.windowWidth - 22, m_iRealFontHeight, 0x99CCCC1Au);
                EnableAlphaTest();
            }
        }
        else
        {
            g_pRenderText->SetTextColor(164, 39, 17, 255);

            if ((*li)->_bStrife)
                g_pRenderText->RenderText(m_StrifePos.x, itemY, I18N::Game::Battle2987, 0, 0, RT3_WRITE_CENTER);

            g_pRenderText->RenderText(m_MapNamePos.x, itemY, (*li)->_ReqInfo.szMainMapName, 0, 0, RT3_WRITE_CENTER);

            _itow(iReqLevel, szText, 10);
            if (iReqLevel > iLevel)
            {
                g_pRenderText->SetTextColor(255, 51, 26, 255);
            }
            else
            {
                g_pRenderText->SetTextColor(164, 39, 17, 255);
            }
            g_pRenderText->RenderText(m_ReqLevelPos.x, itemY, szText, 0, 0, RT3_WRITE_CENTER);

            _itow((*li)->_ReqInfo.iReqZen, szText, 10);
            if ((*li)->_ReqInfo.iReqZen > (int)iZen)
            {
                g_pRenderText->SetTextColor(255, 51, 26, 255);
            }
            else
            {
                g_pRenderText->SetTextColor(164, 39, 17, 255);
            }
            g_pRenderText->RenderText(m_ReqZenPos.x, itemY, szText, 0, 0, RT3_WRITE_CENTER);
        }

        ++li;
        ++visibleIndex;
    }

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->RenderText(m_Pos.x + m_layout.windowWidth / 2, m_layout.closeTop + 1, I18N::Game::Close388, 0, 0, RT3_WRITE_CENTER);
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUIMoveCommandWindow::OpenningProcess()
{
    RefreshDataAndLayout();
    SetScrollOffset(0);
    m_iSelectedMapName = -1;
    m_iSelectedTextIndex = -1;
    SetStrifeMap();
    SettingCanMoveMap();

    m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
}

void SEASON3B::CNewUIMoveCommandWindow::ClosingProcess()
{
}

float SEASON3B::CNewUIMoveCommandWindow::GetLayerDepth()
{
    return 8.3f;
}

void SEASON3B::CNewUIMoveCommandWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_scrollbar_up.tga", IMAGE_MOVECOMMAND_SCROLL_TOP);
    LoadBitmap(L"Interface\\newui_scrollbar_m.tga", IMAGE_MOVECOMMAND_SCROLL_MIDDLE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scrollbar_down.tga", IMAGE_MOVECOMMAND_SCROLL_BOTTOM);
    LoadBitmap(L"Interface\\newui_scroll_on.tga", IMAGE_MOVECOMMAND_SCROLLBAR_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scroll_off.tga", IMAGE_MOVECOMMAND_SCROLLBAR_OFF, GL_LINEAR);
}

void CNewUIMoveCommandWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_TOP);
    DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_MIDDLE);
    DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_BOTTOM);
    DeleteBitmap(IMAGE_MOVECOMMAND_SCROLLBAR_ON);
    DeleteBitmap(IMAGE_MOVECOMMAND_SCROLLBAR_OFF);
}

BOOL CNewUIMoveCommandWindow::IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const
{
    BOOL bInOtherServer = FALSE;

    switch (iFromMapIndex)
    {
    case WD_30BATTLECASTLE:
    case WD_79UNITEDMARKETPLACE:
        bInOtherServer = TRUE;
        break;
    default:
        break;
    }

    switch (iToMapIndex)
    {
    case 24:
    case 44:
        bInOtherServer = TRUE;
        break;
    default:
        break;
    }

    return bInOtherServer;
}

int CNewUIMoveCommandWindow::GetMapIndexFromMovereq(const wchar_t* pszMapName)
{
    if (pszMapName == NULL)
        return -1;

    int iMapIndex = -1;
    std::list<CMoveCommandData::MOVEINFODATA*>::iterator li;
    for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); li++)
    {
        if (wcsicmp((*li)->_ReqInfo.szMainMapName, pszMapName) == 0 || wcsicmp((*li)->_ReqInfo.szSubMapName, pszMapName) == 0)
        {
            iMapIndex = (*li)->_ReqInfo.index;
            break;
        }
    }

    return iMapIndex;
}
