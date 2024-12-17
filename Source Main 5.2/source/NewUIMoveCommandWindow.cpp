//////////////////////////////////////////////////////////////////////
// NewUIMoveCommandWindow.cpp: implementation of the CNewUIMoveCommandWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMoveCommandWindow.h"
#include "NewUISystem.h"
#include "ZzzInterface.h"
#include "ChangeRingManager.h"
#include "KeyGenerater.h"
#include "ServerListManager.h"
#include "ZzzOpenData.h"
#include "MapManager.h"
#include "CharacterManager.h"
#include "DSPlaySound.h"

using namespace SEASON3B;

namespace
{
    const int MapNameCount = 6;

    const std::wstring MapName[MapNameCount] =
    {
        L"Lorencia",
        L"Noria",
        L"Elbeland",
        L"Dungeon",
        L"Devias",
        L"LostTower",
    };

    const bool IsLuckySeal(const std::wstring& name)
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

    memset(&m_ScrollBarPos, 0, sizeof(POINT));
    memset(&m_ScrollBtnStartPos, 0, sizeof(POINT));
    memset(&m_ScrollBtnPos, 0, sizeof(POINT));
    m_iScrollBarHeightPixel = 0;
    m_iRenderStartTextIndex = 0;
    m_iSelectedTextIndex = -1;
    m_iScrollBtnInterval = 0;
    m_iScrollBarMiddleNum = 0;
    m_iScrollBarMiddleRemainderPixel = 0;
    m_iNumPage = 0;
    m_iCurPage = 0;
    m_iMousePosY = 0;
    m_bScrollBtnActive = false;
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

    m_StrifePos.x = m_Pos.x + 20;
    switch (WindowWidth)
    {
    case 640:
        m_MapNameUISize.x = 220; m_MapNamePos.x = m_Pos.x + 62; m_ReqLevelPos.x = m_Pos.x + 119; m_ReqZenPos.x = m_Pos.x + 159;
        break;
    case 800:
        m_MapNameUISize.x = 200; m_MapNamePos.x = m_Pos.x + 69; m_ReqLevelPos.x = m_Pos.x + 129; m_ReqZenPos.x = m_Pos.x + 174;
        break;
    case 1024:
        m_MapNameUISize.x = 180; m_MapNamePos.x = m_Pos.x + 64; m_ReqLevelPos.x = m_Pos.x + 119; m_ReqZenPos.x = m_Pos.x + 159;
        break;
    case 1280:
        m_MapNameUISize.x = 160; m_MapNamePos.x = m_Pos.x + 59; m_ReqLevelPos.x = m_Pos.x + 104; m_ReqZenPos.x = m_Pos.x + 139;
        break;
    case 1366:
        m_MapNameUISize.x = 150; m_MapNamePos.x = m_Pos.x + 56; m_ReqLevelPos.x = m_Pos.x + 101; m_ReqZenPos.x = m_Pos.x + 134;
        break;
    case 1440:
        m_MapNameUISize.x = 140; m_MapNamePos.x = m_Pos.x + 53; m_ReqLevelPos.x = m_Pos.x + 97; m_ReqZenPos.x = m_Pos.x + 129;
        break;
    case 1600:
        m_MapNameUISize.x = 120; m_MapNamePos.x = m_Pos.x + 46; m_ReqLevelPos.x = m_Pos.x + 86; m_ReqZenPos.x = m_Pos.x + 114;
        break;
    case 1680:
        m_MapNameUISize.x = 115; m_MapNamePos.x = m_Pos.x + 44; m_ReqLevelPos.x = m_Pos.x + 83; m_ReqZenPos.x = m_Pos.x + 110;
        break;
    case 1920:
        m_MapNameUISize.x = 110; m_MapNamePos.x = m_Pos.x + 38; m_ReqLevelPos.x = m_Pos.x + 70; m_ReqZenPos.x = m_Pos.x + 93;
        break;
    default:
        // handle unsupported resolutions here
        break;
    }

    m_MapNameUISize.x += 10;

    m_listMoveInfoData = CMoveCommandData::GetInstance()->GetMoveCommandDatalist();
    m_iRealFontHeight = FontHeight * 640 / WindowWidth + 2;

    m_MapNameUISize.y = 60 + (m_iRealFontHeight * MOVECOMMAND_MAX_RENDER_TEXTLINE);

    m_StartUISubjectName.x = m_Pos.x + m_MapNameUISize.x / 2;
    m_StartUISubjectName.y = m_Pos.y + 4;

    m_StartMapNamePos.x = m_Pos.x + 2;
    m_StartMapNamePos.y = m_Pos.y + 38;

    m_StrifePos.y = m_StartMapNamePos.y;

    m_MapNamePos.y = m_StartMapNamePos.y;
    m_ReqLevelPos.y = m_StartMapNamePos.y;
    m_ReqZenPos.y = m_StartMapNamePos.y;
    m_ScrollBarPos.x = m_Pos.x + m_MapNameUISize.x - 14;
    m_ScrollBarPos.y = m_Pos.y + m_StartMapNamePos.y - MOVECOMMAND_SCROLLBAR_TOP_HEIGHT;

    m_ScrollBtnStartPos.x = m_ScrollBarPos.x - (MOVECOMMAND_SCROLLBTN_WIDTH / 2 - MOVECOMMAND_SCROLLBAR_TOP_WIDTH / 2);
    m_ScrollBtnStartPos.y = m_ScrollBarPos.y;
    m_ScrollBtnPos.x = m_ScrollBtnStartPos.x;
    m_ScrollBtnPos.y = m_ScrollBtnStartPos.y;

    m_iScrollBarHeightPixel = MOVECOMMAND_MAX_RENDER_TEXTLINE * m_iRealFontHeight;

    m_iScrollBarMiddleNum = (m_iScrollBarHeightPixel - (MOVECOMMAND_SCROLLBAR_TOP_HEIGHT * 2)) / MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT;
    m_iScrollBarMiddleRemainderPixel = (m_iScrollBarHeightPixel - (MOVECOMMAND_SCROLLBAR_TOP_HEIGHT * 2)) % MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT;

    m_iNumPage = 1 + (m_listMoveInfoData.size() / MOVECOMMAND_MAX_RENDER_TEXTLINE);

    m_iCurPage = 1;

    m_iTotalMoveScrBtnperStep = m_listMoveInfoData.size() - MOVECOMMAND_MAX_RENDER_TEXTLINE;
    m_iRemainMoveScrBtnperStep = m_iTotalMoveScrBtnperStep;
    m_iTotalMoveScrBtnPixel = m_iScrollBarHeightPixel - MOVECOMMAND_SCROLLBTN_HEIGHT;
    m_iRemainMoveScrBtnPixel = m_iTotalMoveScrBtnPixel;
    m_iMinMoveScrBtnPixelperStep = m_iTotalMoveScrBtnPixel / m_iTotalMoveScrBtnperStep;
    m_iMaxMoveScrBtnPixelperStep = m_iMinMoveScrBtnPixelperStep + 1;
    m_iTotalNumMaxMoveScrBtnperStep = m_iTotalMoveScrBtnPixel - (m_iTotalMoveScrBtnperStep * m_iMinMoveScrBtnPixelperStep);
    m_iTotalNumMinMoveScrBtnperStep = m_iTotalMoveScrBtnperStep - m_iTotalNumMaxMoveScrBtnperStep;
    m_icurMoveScrBtnPixelperStep = m_iMaxMoveScrBtnPixelperStep;
    m_iAcumMoveMouseScrollPixel = 0;

    if (m_iNumPage > 1)
    {
        m_bScrollBtnActive = true;
    }

    m_iRenderStartTextIndex = 0;
    m_iRenderEndTextIndex = m_iRenderStartTextIndex + MOVECOMMAND_MAX_RENDER_TEXTLINE;

    if (m_iRenderEndTextIndex > (int)m_listMoveInfoData.size())
    {
        m_iRenderEndTextIndex -= (m_iRenderEndTextIndex - m_listMoveInfoData.size());
    }
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
        if (src.find(GlobalText[260]) != std::wstring::npos) {
            std::wstring temp = GlobalText[260];
            temp += ' ';
            swprintf(lpszStr1, src.c_str());
            lpszStr2 = _wcstok(lpszStr1, temp.c_str());
            if (lpszStr2 == NULL) return false;

            SettingCanMoveMap();
            auto li = m_listMoveInfoData.begin();
            for (int i = 0; i < m_iRenderEndTextIndex; i++, li++) {
                if (!wcscmp(lpszStr2, (*li)->_ReqInfo.szMainMapName)) {
                    if ((*li)->_bCanMove == true) {
                        return IsLuckySeal((*li)->_ReqInfo.szSubMapName);
                    }
                }
            }
            return false;
        }
        else if (src.find(L"/move") != std::wstring::npos) {
            std::wstring temp = L"/move";
            temp += ' ';
            swprintf(lpszStr1, src.c_str());
            lpszStr2 = _wcstok(lpszStr1, temp.c_str());
            if (lpszStr2 == NULL) return false;

            SettingCanMoveMap();
            auto li = m_listMoveInfoData.begin();
            for (int i = 0; i < m_iRenderEndTextIndex; i++, li++) {
                if (!wcsicmp(lpszStr2, (*li)->_ReqInfo.szMainMapName)) {
                    if ((*li)->_bCanMove == true) {
                        return IsLuckySeal((*li)->_ReqInfo.szSubMapName);
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
    m_dwMoveCommandKey = g_KeyGenerater.GenerateKeyValue(m_dwMoveCommandKey);

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

    auto li = m_listMoveInfoData.begin();
    for (int i = 0; i < m_iRenderEndTextIndex; i++, li++)
    {
        if (li == m_listMoveInfoData.end())
        {
            break;
        }
        if (i < m_iRenderStartTextIndex)
        {
            continue;
        }

        (*li)->_bCanMove = false;
        (*li)->_bSelected = false;

        //	if( i < m_iRenderEndTextIndex-1 )	continue;

        iLevel = CharacterAttribute->Level;
        iZen = CharacterMachine->Gold;
        iReqLevel = (*li)->_ReqInfo.iReqLevel;
        iReqZen = (*li)->_ReqInfo.iReqZen;

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

            if (wcscmp((*li)->_ReqInfo.szMainMapName, GlobalText[55]) == 0)
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
                    (*li)->_bCanMove = true;
                }
                else
                {
                    (*li)->_bCanMove = false;
                }
            }
            else if (wcsncmp((*li)->_ReqInfo.szMainMapName, GlobalText[37], wcslen(GlobalText[37])) == 0)
            {
                if (pEquipedHelper->Type == ITEM_HORN_OF_UNIRIA || pEquipedHelper->Type == ITEM_HORN_OF_DINORANT)
                {
                    (*li)->_bCanMove = false;
                }
                else
                {
                    (*li)->_bCanMove = true;
                }
            }
            else if ((g_ServerListManager->IsNonPvP() == true) && (wcscmp((*li)->_ReqInfo.szMainMapName, GlobalText[2686]) == 0))
            {
                (*li)->_bCanMove = false;
            }
            else
            {
                (*li)->_bCanMove = true;
            }
        }

        if ((*li)->_bCanMove && (*li)->_bStrife && 0 == Hero->m_byGensInfluence)
            (*li)->_bCanMove = false;
    }
}

bool SEASON3B::CNewUIMoveCommandWindow::BtnProcess()
{
    int iX, iY;

    if (CheckMouseIn(m_ScrollBtnPos.x, m_ScrollBtnPos.y, MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT))
    {
        if (IsPress(VK_LBUTTON))
        {
            m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_CLICKED;
            m_iMousePosY = MouseY;
            m_iAcumMoveMouseScrollPixel = 0;
        }
    }

    if (IsRelease(VK_LBUTTON))
    {
        m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
        m_iAcumMoveMouseScrollPixel = 0;
    }

    //	if( CheckMouseIn( 0, m_ScrollBarPos.y, 640, m_iTotalMoveScrBtnPixel) )
    //	{
    if (m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED && m_icurMoveScrBtnPixelperStep > 0)
    {
        //MoveScrollBtn(MouseY-m_iMousePosY);
        int iMoveValue = MouseY - m_iMousePosY;

        if (iMoveValue < 0)
        {
            if (MouseY <= m_ScrollBtnPos.y + (MOVECOMMAND_SCROLLBTN_HEIGHT / 2))
            {
                if (-(iMoveValue) > (m_iTotalMoveScrBtnPixel - m_iRemainMoveScrBtnPixel))
                {
                    iMoveValue = -(m_iTotalMoveScrBtnPixel - m_iRemainMoveScrBtnPixel);
                }
                ScrollUp(-iMoveValue);
            }
        }
        else if (iMoveValue > 0)
        {
            if (MouseY >= m_ScrollBtnPos.y + (MOVECOMMAND_SCROLLBTN_HEIGHT / 2))
            {
                if (iMoveValue > m_iRemainMoveScrBtnPixel)
                {
                    iMoveValue = m_iRemainMoveScrBtnPixel;
                }
                ScrollDown(iMoveValue);
            }
        }
        m_iMousePosY = MouseY;
        //UpdateScrolling();
    }
    //	}
    // 	else
    // 	{
    // 		m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
    // 		m_iAcumMoveMouseScrollPixel = 0;
    // 	}
    if (CheckMouseIn(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y) && IsPress(VK_LBUTTON))
    {
        SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
    }

    SettingCanMoveMap();

    if (CheckMouseIn(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y))
    {
        if (m_icurMoveScrBtnPixelperStep > 0)
        {
            if (MouseWheel > 0)
            {
                ScrollUp(m_icurMoveScrBtnPixelperStep);
            }
            else if (MouseWheel < 0)
            {
                ScrollDown(m_icurMoveScrBtnPixelperStep);
            }
        }
        MouseWheel = 0;

        auto li = m_listMoveInfoData.begin();
        int iCurRenderTextIndex = 0;
        for (int i = 0; i < m_iRenderEndTextIndex; i++, li++)
        {
            if (li == m_listMoveInfoData.end())
            {
                break;
            }

            if (i < m_iRenderStartTextIndex)
            {
                continue;
            }
            iX = m_StartMapNamePos.x;
            iY = m_StartMapNamePos.y + (m_iRealFontHeight * iCurRenderTextIndex);
            if (CheckMouseIn(iX, iY, m_MapNameUISize.x - 22, m_iRealFontHeight))
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

            iCurRenderTextIndex++;

            if (SEASON3B::IsRelease(VK_LBUTTON) && CheckMouseIn(3, m_MapNameUISize.y - m_iRealFontHeight - 6, m_MapNameUISize.x - 5, m_iRealFontHeight))
            {
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_MOVEMAP);
                return true;
            }
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
    if (!IsVisible())
    {
        return true;
    }

    UpdateScrolling();

    return true;
}

void SEASON3B::CNewUIMoveCommandWindow::ScrollUp(int iMoveValue)
{
    if (m_iRemainMoveScrBtnperStep < m_iTotalMoveScrBtnperStep)
    {
        int iMovePixel = 0;
        m_iAcumMoveMouseScrollPixel -= iMoveValue;
        if ((-m_iAcumMoveMouseScrollPixel) < m_icurMoveScrBtnPixelperStep)
        {
            return;
        }
        else
        {
            //m_iAcumMoveMouseScrollPixel = 0;
            RecursiveCalcScroll(m_iAcumMoveMouseScrollPixel, &iMovePixel, false);

            g_ConsoleDebug->Write(MCD_NORMAL, L"m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);

            m_ScrollBtnPos.y += iMovePixel;
            m_iAcumMoveMouseScrollPixel -= iMovePixel;

            g_ConsoleDebug->Write(MCD_NORMAL, L"m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);
            g_ConsoleDebug->Write(MCD_NORMAL, L"iMoveValue : (%d)", -iMoveValue);
            g_ConsoleDebug->Write(MCD_NORMAL, L"m_iRemainMoveScrBtnPixel : (%d)", m_iRemainMoveScrBtnPixel);
            g_ConsoleDebug->Write(MCD_NORMAL, L"m_icurMoveScrBtnPixelperStep : (%d)", m_icurMoveScrBtnPixelperStep);
            g_ConsoleDebug->Write(MCD_NORMAL, L"m_iRemainMoveScrBtnperStep : (%d)", m_iRemainMoveScrBtnperStep);
            g_ConsoleDebug->Write(MCD_NORMAL, L"m_iAcumMoveMouseScrollPixel : (%d)", m_iAcumMoveMouseScrollPixel);
        }
    }
}

void SEASON3B::CNewUIMoveCommandWindow::ScrollDown(int iMoveValue)
{
    if (m_iRemainMoveScrBtnperStep > 0)
    {
        int iMovePixel = 0;
        m_iAcumMoveMouseScrollPixel += iMoveValue;
        if (m_iAcumMoveMouseScrollPixel < m_icurMoveScrBtnPixelperStep)
        {
            return;
        }
        else
        {
            RecursiveCalcScroll(m_iAcumMoveMouseScrollPixel, &iMovePixel, true);

            g_ConsoleDebug->Write(MCD_NORMAL, L"m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);

            m_iAcumMoveMouseScrollPixel -= iMovePixel;
            m_ScrollBtnPos.y += iMovePixel;

            g_ConsoleDebug->Write(MCD_NORMAL, L"m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);
            g_ConsoleDebug->Write(MCD_NORMAL, L"iMoveValue : (%d)", iMoveValue);
            g_ConsoleDebug->Write(MCD_NORMAL, L"m_iRemainMoveScrBtnPixel : (%d)", m_iRemainMoveScrBtnPixel);
            g_ConsoleDebug->Write(MCD_NORMAL, L"m_icurMoveScrBtnPixelperStep : (%d)", m_icurMoveScrBtnPixelperStep);
            g_ConsoleDebug->Write(MCD_NORMAL, L"m_iRemainMoveScrBtnperStep : (%d)", m_iRemainMoveScrBtnperStep);
            g_ConsoleDebug->Write(MCD_NORMAL, L"m_iAcumMoveMouseScrollPixel : (%d)", m_iAcumMoveMouseScrollPixel);
        }
    }
}

void SEASON3B::CNewUIMoveCommandWindow::RecursiveCalcScroll(IN int piScrollValue, OUT int* piMovePixel, bool bSign /* = true */)
{
    if (bSign == true)
    { // DownScroll
        if (m_iRemainMoveScrBtnperStep > 0)
        {
            m_iRemainMoveScrBtnperStep--;
            m_iRemainMoveScrBtnPixel -= m_icurMoveScrBtnPixelperStep;
            piScrollValue -= m_icurMoveScrBtnPixelperStep;
            (*piMovePixel) += m_icurMoveScrBtnPixelperStep;

            if (m_iRemainMoveScrBtnperStep > m_iTotalNumMinMoveScrBtnperStep)
            {
                m_icurMoveScrBtnPixelperStep = m_iMaxMoveScrBtnPixelperStep;
            }
            else
            {
                m_icurMoveScrBtnPixelperStep = m_iMinMoveScrBtnPixelperStep;
            }

            if (piScrollValue >= m_icurMoveScrBtnPixelperStep)
            {
                RecursiveCalcScroll(piScrollValue, piMovePixel, bSign);
            }
        }
        else
        {
            (*piMovePixel) = piScrollValue;
        }
    }
    else
    { // UpScroll
        if (m_iRemainMoveScrBtnperStep < m_iTotalMoveScrBtnperStep)
        {
            m_iRemainMoveScrBtnperStep++;
            m_iRemainMoveScrBtnPixel += m_icurMoveScrBtnPixelperStep;
            piScrollValue += m_icurMoveScrBtnPixelperStep;
            (*piMovePixel) -= m_icurMoveScrBtnPixelperStep;

            if (m_iRemainMoveScrBtnperStep >= m_iTotalNumMinMoveScrBtnperStep)
            {
                m_icurMoveScrBtnPixelperStep = m_iMaxMoveScrBtnPixelperStep;
            }
            else
            {
                m_icurMoveScrBtnPixelperStep = m_iMinMoveScrBtnPixelperStep;
            }

            if ((-piScrollValue) >= m_icurMoveScrBtnPixelperStep)
            {
                RecursiveCalcScroll(piScrollValue, piMovePixel, bSign);
            }
        }
        else
        {
            (*piMovePixel) = piScrollValue;
        }
    }

    return;
}

void SEASON3B::CNewUIMoveCommandWindow::UpdateScrolling()
{
    m_iRenderStartTextIndex = m_iTotalMoveScrBtnperStep - m_iRemainMoveScrBtnperStep;

    m_iRenderEndTextIndex = m_iRenderStartTextIndex + MOVECOMMAND_MAX_RENDER_TEXTLINE;

    if (m_iRenderEndTextIndex > (int)m_listMoveInfoData.size())
    {
        m_iRenderEndTextIndex -= (m_iRenderEndTextIndex - m_listMoveInfoData.size());
    }
}

void SEASON3B::CNewUIMoveCommandWindow::RenderFrame()
{
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);

    RenderColor((float)m_Pos.x, (float)m_Pos.y, (float)m_MapNameUISize.x, (float)m_MapNameUISize.y);

    glColor4f(0.6f, 0.f, 0.f, 1.f);

    RenderColor(m_StartMapNamePos.x, m_MapNameUISize.y - m_iRealFontHeight - 6, m_MapNameUISize.x - 5, m_iRealFontHeight);

    glColor4f(1.f, 1.f, 1.f, 1.f);
    EnableAlphaTest();

    RenderImage(IMAGE_MOVECOMMAND_SCROLL_TOP, m_ScrollBarPos.x, m_ScrollBarPos.y, MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT);		// TOP

    int icntText = 0;
    for (int i = 0; i < m_iScrollBarMiddleNum; i++)
    {
        icntText = i;
        RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x,
            m_ScrollBarPos.y + MOVECOMMAND_SCROLLBAR_TOP_HEIGHT + (i * MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
            MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT);	// MIDDLE
    }
    if (m_iScrollBarMiddleRemainderPixel > 0)
    {
        RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x,
            m_ScrollBarPos.y + MOVECOMMAND_SCROLLBAR_TOP_HEIGHT + (icntText * MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
            MOVECOMMAND_SCROLLBAR_TOP_WIDTH, m_iScrollBarMiddleRemainderPixel);	// MIDDLE 나머지
    }

    RenderImage(IMAGE_MOVECOMMAND_SCROLL_BOTTOM, m_ScrollBarPos.x, m_ScrollBarPos.y + m_iScrollBarHeightPixel - MOVECOMMAND_SCROLLBAR_TOP_HEIGHT,
        MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT);		// BOTTOM

    if (m_bScrollBtnActive == true)
    {
        if (m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED)
        {
            glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
        }
        RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_ON, m_ScrollBtnPos.x, m_ScrollBtnPos.y,
            MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT);
    }
    else
    {
        RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_OFF, m_ScrollBtnPos.x, m_ScrollBtnPos.y,
            MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT);
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(255, 204, 26, 255);
    g_pRenderText->RenderText(m_StartUISubjectName.x, m_StartUISubjectName.y, GlobalText[933], 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(127, 178, 255, 255);
    g_pRenderText->RenderText(m_StrifePos.x, m_StartUISubjectName.y + 20, GlobalText[2988], 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->RenderText(m_MapNamePos.x, m_StartUISubjectName.y + 20, GlobalText[934], 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->RenderText(m_ReqLevelPos.x, m_StartUISubjectName.y + 20, GlobalText[935], 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->RenderText(m_ReqZenPos.x, m_StartUISubjectName.y + 20, GlobalText[936], 0, 0, RT3_WRITE_CENTER);
}

bool SEASON3B::CNewUIMoveCommandWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    RenderFrame();

    auto li = m_listMoveInfoData.begin();
    int iX, iY;

    int iLevel = CharacterAttribute->Level;
    DWORD iZen = CharacterMachine->Gold;
    int iReqLevel;
    wchar_t szText[24];

    int iCurRenderTextIndex = 0;
    for (int i = 0; i < m_iRenderEndTextIndex; i++, li++)
    {
        if (li == m_listMoveInfoData.end())
        {
            break;
        }

        if (i < m_iRenderStartTextIndex)
        {
            continue;
        }

        iX = m_StartMapNamePos.x;
        iY = m_StartMapNamePos.y + (m_iRealFontHeight * iCurRenderTextIndex);

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
                g_pRenderText->RenderText(m_StrifePos.x, iY, GlobalText[2987], 0, 0, RT3_WRITE_CENTER);
            g_pRenderText->RenderText(m_MapNamePos.x, iY, (*li)->_ReqInfo.szMainMapName, 0, 0, RT3_WRITE_CENTER);
            _itow(iReqLevel, szText, 10);
            g_pRenderText->RenderText(m_ReqLevelPos.x, iY, szText, 0, 0, RT3_WRITE_CENTER);
            _itow((*li)->_ReqInfo.iReqZen, szText, 10);
            g_pRenderText->RenderText(m_ReqZenPos.x, iY, szText, 0, 0, RT3_WRITE_CENTER);

            if ((*li)->_bSelected == true)
            {
                glColor4f(0.8f, 0.8f, 0.1f, 0.6f);
                RenderColor(iX, iY - 1, m_MapNameUISize.x - 22, m_iRealFontHeight);
                glColor4f(1.f, 1.f, 1.f, 1.f);
                EnableAlphaTest();
            }
        }
        else
        {
            g_pRenderText->SetTextColor(164, 39, 17, 255);

            if ((*li)->_bStrife)
                g_pRenderText->RenderText(m_StrifePos.x, iY, GlobalText[2987], 0, 0, RT3_WRITE_CENTER);

            g_pRenderText->RenderText(m_MapNamePos.x, iY, (*li)->_ReqInfo.szMainMapName, 0, 0, RT3_WRITE_CENTER);

            _itow(iReqLevel, szText, 10);
            if (iReqLevel > iLevel)
            {
                g_pRenderText->SetTextColor(255, 51, 26, 255);
            }
            else
            {
                g_pRenderText->SetTextColor(164, 39, 17, 255);
            }
            g_pRenderText->RenderText(m_ReqLevelPos.x, iY, szText, 0, 0, RT3_WRITE_CENTER);

            _itow((*li)->_ReqInfo.iReqZen, szText, 10);
            if ((*li)->_ReqInfo.iReqZen > (int)iZen)
            {
                g_pRenderText->SetTextColor(255, 51, 26, 255);
            }
            else
            {
                g_pRenderText->SetTextColor(164, 39, 17, 255);
            }
            g_pRenderText->RenderText(m_ReqZenPos.x, iY, szText, 0, 0, RT3_WRITE_CENTER);
        }

        iCurRenderTextIndex++;
    }

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->RenderText(m_MapNameUISize.x / 2, m_MapNameUISize.y - m_iRealFontHeight - 5, GlobalText[1002], 0, 0, RT3_WRITE_CENTER);
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUIMoveCommandWindow::OpenningProcess()
{
    SetPos(m_Pos.x, m_Pos.y);
    SetStrifeMap();
    SettingCanMoveMap();

    m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
    m_ScrollBtnPos.y = m_ScrollBtnStartPos.y;
    m_iRenderStartTextIndex = 0;

    m_iRenderEndTextIndex = m_iRenderStartTextIndex + MOVECOMMAND_MAX_RENDER_TEXTLINE;

    if (m_iRenderEndTextIndex > (int)m_listMoveInfoData.size())
    {
        m_iRenderEndTextIndex -= (m_iRenderEndTextIndex - m_listMoveInfoData.size());
    }
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