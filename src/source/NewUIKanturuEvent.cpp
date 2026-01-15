//////////////////////////////////////////////////////////////////////
// NewUIKanturu2ndEnterNpc.cpp: implementation of the CNewUIKanturu2ndEnterNpc class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIKanturuEvent.h"
#include "NewUICommonMessageBox.h"
#include "NewUISystem.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"

#include "ChangeRingManager.h"
#include "CDirection.h"
#include "DSPlaySound.h"

SEASON3B::CNewUIKanturu2ndEnterNpc::CNewUIKanturu2ndEnterNpc()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_pNpcObject = NULL;
    m_dwRefreshTime = 0;
    m_dwRefreshButtonGapTime = 0;

    Initialize();
}

SEASON3B::CNewUIKanturu2ndEnterNpc::~CNewUIKanturu2ndEnterNpc()
{
    Release();
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::Initialize()
{
    m_bNpcAnimation = false;
    m_bEnterRequest = false;

    m_iStateTextNum = 0;
    ZeroMemory(m_strSubject, sizeof(m_strSubject));
    for (int i = 0; i < KANTURU2ND_STATETEXT_MAX; i++)
    {
        ZeroMemory(m_strStateText[i], sizeof(m_strStateText[i]));
    }
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC, this);

    SetPos(x, y);

    LoadImages();

    SetButtonInfo();

    Show(false);

    return true;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::UpdateMouseEvent()
{
    if (BtnProcess() == true)
    {
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, KANTURU2ND_ENTER_WINDOW_WIDTH, KANTURU2ND_ENTER_WINDOW_HEIGHT))
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::Update()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC) == true)
    {
        if (timeGetTime() - m_dwRefreshTime > KANTURU2ND_REFRESH_GAPTIME)
        {
            SendRequestKanturu3rdInfo();
        }
    }

    return true;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::Render()
{
    EnableAlphaTest();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();

    RenderTexts();

    RenderButtons();

    return true;
}

float SEASON3B::CNewUIKanturu2ndEnterNpc::GetLayerDepth()
{
    return 10.1f;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetNpcObject(OBJECT* pObj)
{
    m_pNpcObject = pObj;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::IsNpcAnimation()
{
    return m_bNpcAnimation;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetNpcAnimation(bool bValue)
{
    m_bNpcAnimation = bValue;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::IsEnterRequest()
{
    return m_bEnterRequest;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetEnterRequest(bool bValue)
{
    m_bEnterRequest = bValue;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::CreateMessageBox(BYTE btResult)
{
    wchar_t strMessage[256];
    if (btResult == POPUP_FAILED || btResult == POPUP_FAILED2)
    {
        wcscpy(strMessage, GlobalText[2170]);
    }
    else if (btResult == POPUP_UNIRIA)
    {
        wcscpy(strMessage, GlobalText[569]);
    }
    else if (btResult == POPUP_CHANGERING)
    {
        wcscpy(strMessage, GlobalText[2175]);
    }
    else if (btResult == POPUP_NOT_HELPER)
    {
        wcscpy(strMessage, GlobalText[2176]);
    }
    else
    {
        wcscpy(strMessage, GlobalText[2170 + btResult]);
    }

    SEASON3B::CreateOkMessageBox(strMessage);
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::ReceiveKanturu3rdInfo(BYTE btState, BYTE btDetailState, BYTE btEnter, BYTE btUserCount, int iRemainTime)
{
    if (m_pNpcObject && m_pNpcObject->CurrentAction == KANTURU2ND_NPC_ANI_ROT)
    {
        return;
    }

    if (g_MessageBox->IsEmpty() == false)
    {
        return;
    }

    Initialize();

    m_byState = btState;

    if (btEnter == 1)
    {
        m_BtnEnter.UnLock();
        m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_BtnEnter.ChangeTextColor(RGBA(255, 255, 255, 255));
    }
    else
    {
        m_BtnEnter.Lock();
        m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
        m_BtnEnter.ChangeTextColor(RGBA(100, 100, 100, 255));
    }

    if (btState == KANTURU_STATE_TOWER)
    {
        if (btDetailState == KANTURU_TOWER_REVITALIXATION || btDetailState == KANTURU_TOWER_NOTIFY)
        {
            wcscpy(m_strSubject, GlobalText[2149]);
            wcscpy(m_strStateText[0], GlobalText[2150]);
            swprintf(m_strStateText[1], GlobalText[2151], iRemainTime / 3600);
            m_iStateTextNum = 2;
        }
        else
        {
            wcscpy(m_strSubject, GlobalText[2174]);
            wcscpy(m_strStateText[0], GlobalText[2160]);
            wcscpy(m_strStateText[1], GlobalText[2161]);
            m_iStateTextNum = 2;
        }
    }
    else if (btState == KANTURU_STATE_MAYA_BATTLE)
    {
        if (btDetailState != KANTURU_MAYA_DIRECTION_STANBY1
            && btDetailState != KANTURU_MAYA_DIRECTION_STANBY2
            && btDetailState != KANTURU_MAYA_DIRECTION_STANBY3)
        {
            wcscpy(m_strSubject, GlobalText[2152]);
            swprintf(m_strStateText[0], GlobalText[2153], btUserCount);
        }
        else
        {
            wcscpy(m_strSubject, GlobalText[2163]);

            if (btDetailState == KANTURU_MAYA_DIRECTION_STANBY1)
            {
                if (btUserCount < 15)
                {
                    wcscpy(m_strStateText[0], GlobalText[2164]);
                }
                else if (btUserCount == 15)
                {
                    wcscpy(m_strStateText[0], GlobalText[2172]);
                }
                else
                {
                    if (m_BtnEnter.IsLock() == false)
                    {
                        wcscpy(m_strStateText[0], GlobalText[2164]);
                    }
                    else
                    {
                        wcscpy(m_strStateText[0], GlobalText[2172]);
                    }
                }
                m_iStateTextNum = 1;
            }
            else if (btDetailState == KANTURU_MAYA_DIRECTION_STANBY2)
            {
                if (btUserCount < 15)
                {
                    swprintf(m_strStateText[0], GlobalText[2165], btUserCount);
                    swprintf(m_strStateText[1], GlobalText[2167], 15 - btUserCount);
                    m_iStateTextNum = 2;
                }
                else if (btUserCount == 15)
                {
                    wcscpy(m_strStateText[0], GlobalText[2168]);
                    m_iStateTextNum = 1;
                }
            }
            else if (btDetailState == KANTURU_MAYA_DIRECTION_STANBY3)
            {
                if (btUserCount < 15)
                {
                    swprintf(m_strStateText[0], GlobalText[2166], btUserCount);
                    swprintf(m_strStateText[1], GlobalText[2167], 15 - btUserCount);
                    m_iStateTextNum = 2;
                }
                else if (btUserCount == 15)
                {
                    wcscpy(m_strStateText[0], GlobalText[2168]);
                    m_iStateTextNum = 1;
                }
            }
            else
            {
                if (m_BtnEnter.IsLock() == false)
                {
                    wcscpy(m_strStateText[0], GlobalText[2164]);

                    m_iStateTextNum = 1;
                }
            }
        }

        if (btDetailState == KANTURU_MAYA_DIRECTION_NOTIFY || btDetailState == KANTURU_MAYA_DIRECTION_MONSTER1 || btDetailState == KANTURU_MAYA_DIRECTION_MAYA1
            || btDetailState == KANTURU_MAYA_DIRECTION_END_MAYA1 || btDetailState == KANTURU_MAYA_DIRECTION_ENDCYCLE_MAYA1)
        {
            swprintf(m_strStateText[1], GlobalText[2154], btUserCount);
            m_iStateTextNum = 2;
        }
        else if (btDetailState == KANTURU_MAYA_DIRECTION_MONSTER2 || btDetailState == KANTURU_MAYA_DIRECTION_MAYA2
            || btDetailState == KANTURU_MAYA_DIRECTION_END_MAYA2 || btDetailState == KANTURU_MAYA_DIRECTION_ENDCYCLE_MAYA2)
        {
            swprintf(m_strStateText[1], GlobalText[2155], btUserCount);
            m_iStateTextNum = 2;
        }
        else if (btDetailState == KANTURU_MAYA_DIRECTION_MONSTER3 || btDetailState == KANTURU_MAYA_DIRECTION_MAYA3
            || btDetailState == KANTURU_MAYA_DIRECTION_END_MAYA3 || btDetailState == KANTURU_MAYA_DIRECTION_ENDCYCLE_MAYA3)
        {
            swprintf(m_strStateText[1], GlobalText[2156], btUserCount);
            m_iStateTextNum = 2;
        }
        else if (btDetailState == KANTURU_MAYA_DIRECTION_NONE || btDetailState == KANTURU_MAYA_DIRECTION_END
            || btDetailState == KANTURU_MAYA_DIRECTION_ENDCYCLE)
        {
            m_iStateTextNum = 1;
        }
    }
    else if (btState == KANTURU_STATE_NIGHTMARE_BATTLE)
    {
        wcscpy(m_strSubject, GlobalText[2152]);
        swprintf(m_strStateText[0], GlobalText[2153], btUserCount);
        swprintf(m_strStateText[1], GlobalText[2157], btUserCount);
        m_iStateTextNum = 2;
    }
    else if (btState == KANTURU_STATE_STANDBY)
    {
        wcscpy(m_strSubject, GlobalText[2158]);
        if (btDetailState == 1)	// STANBY_START
        {
            swprintf(m_strStateText[0], GlobalText[2159], iRemainTime / 60);
        }
        else // STANBY_NONE || STANBY_NOTIFY || STANBY_END || STANBY_ENDCYCLE
        {
            swprintf(m_strStateText[0], GlobalText[2162]);
        }
        swprintf(m_strStateText[1], GlobalText[2160]);
        swprintf(m_strStateText[2], GlobalText[2161]);
        m_iStateTextNum = 3;
    }
    else
    {
        wcscpy(m_strSubject, GlobalText[2170]);
    }

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC) == false)
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);
    }
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::ReceiveKanturu3rdEnter(BYTE btResult)
{
    m_bEnterRequest = false;
    CreateMessageBox(btResult);

    m_pNpcObject->AnimationFrame = 0;
    m_bNpcAnimation = false;
    SetAction(m_pNpcObject, KANTURU2ND_NPC_ANI_STOP);

    DeleteJoint(BITMAP_JOINT_ENERGY, NULL);

    g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SendRequestKanturu3rdInfo()
{
    SocketClient->ToGameServer()->SendKanturuInfoRequest();
    m_dwRefreshTime = timeGetTime();
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SendRequestKanturu3rdEnter()
{
    SocketClient->ToGameServer()->SendKanturuEnterRequest();
    m_bEnterRequest = true;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_top.tga", IMAGE_KANTURU2ND_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_middle.tga", IMAGE_KANTURU2ND_MIDDLE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_bottom.tga", IMAGE_KANTURU2ND_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_KANTURU2ND_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_very_small.tga", IMAGE_KANTURU2ND_BTN, GL_LINEAR);
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::UnloadImages()
{
    DeleteBitmap(IMAGE_KANTURU2ND_TOP);
    DeleteBitmap(IMAGE_KANTURU2ND_MIDDLE);
    DeleteBitmap(IMAGE_KANTURU2ND_BOTTOM);
    DeleteBitmap(IMAGE_KANTURU2ND_BACK);
    DeleteBitmap(IMAGE_KANTURU2ND_BTN);
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetButtonInfo()
{
    m_BtnRefresh.ChangeText(GlobalText[2148]);
    m_BtnRefresh.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnRefresh.ChangeButtonImgState(true, IMAGE_KANTURU2ND_BTN, true);
    m_BtnRefresh.ChangeButtonInfo(m_Pos.x + 17, m_Pos.y + 220, 53, 23);
    m_BtnRefresh.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnRefresh.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));

    m_BtnEnter.ChangeText(GlobalText[2147]);
    m_BtnEnter.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnEnter.ChangeButtonImgState(true, IMAGE_KANTURU2ND_BTN, true);
    m_BtnEnter.ChangeButtonInfo(m_Pos.x + 87, m_Pos.y + 220, 53, 23);
    m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnEnter.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));

    m_BtnClose.ChangeText(GlobalText[1002]);
    m_BtnClose.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnClose.ChangeButtonImgState(true, IMAGE_KANTURU2ND_BTN, true);
    m_BtnClose.ChangeButtonInfo(m_Pos.x + 157, m_Pos.y + 220, 53, 23);
    m_BtnClose.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnClose.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::BtnProcess()
{
    if (m_BtnRefresh.IsLock() == true)
    {
        if (timeGetTime() - m_dwRefreshButtonGapTime > KANTURU2ND_REFRESHBUTTON_GAPTIME)
        {
            m_BtnRefresh.UnLock();
            m_BtnRefresh.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
            m_BtnRefresh.ChangeTextColor(RGBA(255, 255, 255, 255));
        }
    }
    else if (m_BtnRefresh.UpdateMouseEvent() == true)
    {
        SendRequestKanturu3rdInfo();

        m_BtnRefresh.Lock();
        m_BtnRefresh.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
        m_BtnRefresh.ChangeTextColor(RGBA(100, 100, 100, 255));

        m_dwRefreshButtonGapTime = timeGetTime();
        return true;
    }

    if (m_BtnEnter.UpdateMouseEvent() == true)
    {
        if (m_pNpcObject)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);

            if (m_byState == KANTURU_STATE_TOWER)
            {
                SetAction(m_pNpcObject, KANTURU2ND_NPC_ANI_ROT);
                m_bNpcAnimation = true;
                return true;
            }

            ITEM* pItemHelper, * pItemRingLeft, * pItemRingRight, * pItemWing;
            pItemHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
            pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
            pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
            pItemWing = &CharacterMachine->Equipment[EQUIPMENT_WING];

            if (pItemHelper->Type == ITEM_HORN_OF_UNIRIA)
            {
                CreateMessageBox(POPUP_UNIRIA);
                return true;
            }

            if (g_ChangeRingMgr->CheckChangeRing(pItemRingLeft->Type)
                || g_ChangeRingMgr->CheckChangeRing(pItemRingRight->Type))
            {
                CreateMessageBox(POPUP_CHANGERING);
                return true;
            }

            if (!((pItemWing->Type >= ITEM_WINGS_OF_ELF && pItemWing->Type <= ITEM_WINGS_OF_DARKNESS)
                || (pItemWing->Type >= ITEM_WING_OF_STORM && pItemWing->Type <= ITEM_WING_OF_DIMENSION)
                || (ITEM_WING + 130 <= pItemWing->Type && pItemWing->Type <= ITEM_WING + 134)
                || pItemHelper->Type == ITEM_HORN_OF_DINORANT
                || pItemHelper->Type == ITEM_DARK_HORSE_ITEM
                || pItemWing->Type == ITEM_CAPE_OF_LORD
                || pItemHelper->Type == ITEM_HORN_OF_FENRIR
                || (pItemWing->Type >= ITEM_CAPE_OF_FIGHTER && pItemWing->Type <= ITEM_CAPE_OF_OVERRULE)
                || (pItemWing->Type == ITEM_WING + 135)))
            {
                CreateMessageBox(POPUP_NOT_HELPER);
                return true;
            }

            if (pItemRingLeft->Type == ITEM_MOONSTONE_PENDANT || pItemRingRight->Type == ITEM_MOONSTONE_PENDANT)
            {
                SetAction(m_pNpcObject, KANTURU2ND_NPC_ANI_ROT);
                m_bNpcAnimation = true;
            }
            else
            {
                CreateMessageBox(POPUP_NOT_MUNSTONE);
                return true;
            }
        }

        return true;
    }

    if (m_BtnClose.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);

        return true;
    }
    return false;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::RenderFrame()
{
    float x, y, width, height;

    x = m_Pos.x; y = m_Pos.y + 2.f, width = KANTURU2ND_ENTER_WINDOW_WIDTH - MSGBOX_BACK_BLANK_WIDTH; height = KANTURU2ND_ENTER_WINDOW_HEIGHT - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(IMAGE_KANTURU2ND_BACK, x, y, width, height);

    x = m_Pos.x; y = m_Pos.y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(IMAGE_KANTURU2ND_TOP, x, y, width, height);

    x = m_Pos.x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 10; ++i)
    {
        RenderImage(IMAGE_KANTURU2ND_MIDDLE, x, y, width, height);
        y += height;
    }

    x = m_Pos.x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(IMAGE_KANTURU2ND_BOTTOM, x, y, width, height);
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::RenderButtons()
{
    m_BtnEnter.Render();
    m_BtnRefresh.Render();
    m_BtnClose.Render();
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::RenderTexts()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(0xFF49B0FF);
    g_pRenderText->SetBgColor(0);

    int iTextY = m_Pos.y + 30;
    int iLine;
    wchar_t strTemp[3][52];
    ZeroMemory(strTemp, sizeof(strTemp));
    iLine = SeparateTextIntoLines(m_strSubject, strTemp[0], 3, 52);
    for (int i = 0; i < iLine; i++)
    {
        g_pRenderText->RenderText(m_Pos.x, iTextY, strTemp[i], KANTURU2ND_ENTER_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
        iTextY += 12;
    }

    iTextY += 20;

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(0xFF61F191);

    for (int i = 0; i < m_iStateTextNum; i++)
    {
        iLine = SeparateTextIntoLines(m_strStateText[i], strTemp[0], 3, 52);
        for (int j = 0; j < iLine; j++)
        {
            g_pRenderText->RenderText(m_Pos.x, iTextY, strTemp[j], KANTURU2ND_ENTER_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
            iTextY += 12;
        }

        iTextY += 15;

        g_pRenderText->SetTextColor(CLRDW_BR_YELLOW);

        ZeroMemory(strTemp[i], sizeof(strTemp[i]));
    }
}

SEASON3B::CNewUIKanturuInfoWindow::CNewUIKanturuInfoWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;

    m_iMinute = 0;
    m_iSecond = 0;
    m_dwSyncTime = 0;
}

SEASON3B::CNewUIKanturuInfoWindow::~CNewUIKanturuInfoWindow()
{
    Release();
}

bool SEASON3B::CNewUIKanturuInfoWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_KANTURU_INFO, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void SEASON3B::CNewUIKanturuInfoWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIKanturuInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUIKanturuInfoWindow::UpdateMouseEvent()
{
    return true;
}

bool SEASON3B::CNewUIKanturuInfoWindow::UpdateKeyEvent()
{
    return true;
}

bool SEASON3B::CNewUIKanturuInfoWindow::Update()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU_INFO))
    {
        if (M39Kanturu3rd::IsInKanturu3rd() == false)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU_INFO);
        }
    }

    return true;
}

bool SEASON3B::CNewUIKanturuInfoWindow::Render()
{
    EnableAlphaTest();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();

    RenderInfo();

    return true;
}

void SEASON3B::CNewUIKanturuInfoWindow::RenderFrame()
{
    RenderImage(IMAGE_KANTURUINFO_WINDOW, m_Pos.x, m_Pos.y, 99.f, 78.f);
}

void SEASON3B::CNewUIKanturuInfoWindow::RenderInfo()
{
    g_pRenderText->SetFont(g_hFontBold);

    wchar_t strText[256];
    swprintf(strText, GlobalText[2180], UserCount);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(134, 134, 199, 255);
    g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 15, strText);

    if (g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA1
        || g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA2
        || g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA3)
    {
        g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 35, GlobalText[2182]);
    }
    else
    {
        swprintf(strText, GlobalText[2183], MonsterCount);
        g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 35, strText);
    }

    int iCurrentTime = (GetTickCount() - m_dwSyncTime) / 1000;
    int iPastSecond = m_iSecond - iCurrentTime;

    m_iMinute = iPastSecond / 60;
    int iSecond;

    if (m_iMinute <= 0)
    {
        iSecond = 0;
    }
    else
    {
        iSecond = iPastSecond % (60 * m_iMinute);
    }

    static DWORD dwTime = timeGetTime();
    static bool bRender = true;
    if (timeGetTime() - dwTime > 500)
    {
        dwTime = timeGetTime();
        bRender = !bRender;
    }

    if (bRender)
    {
        g_pRenderText->RenderText(m_Pos.x + 48, m_Pos.y + 57, L":");
    }

    glColor3f(134.f / 255.f, 134.f / 255.f, 199.f / 255.f);
    SEASON3B::RenderNumber(m_Pos.x + 35, m_Pos.y + 55, m_iMinute, 1.f);
    SEASON3B::RenderNumber(m_Pos.x + 65, m_Pos.y + 55, iSecond, 1.f);
}

float SEASON3B::CNewUIKanturuInfoWindow::GetLayerDepth()
{
    return 1.92f;
}

float SEASON3B::CNewUIKanturuInfoWindow::GetKeyEventOrder()
{
    return 9.1f;
}

void SEASON3B::CNewUIKanturuInfoWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_Figure_kantru.tga", IMAGE_KANTURUINFO_WINDOW, GL_LINEAR);
}

void SEASON3B::CNewUIKanturuInfoWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_KANTURUINFO_WINDOW);
}

void SEASON3B::CNewUIKanturuInfoWindow::SetTime(int iTimeLimit)
{
    m_iMinute = 0;
    m_iSecond = iTimeLimit / 1000;
    m_dwSyncTime = GetTickCount();
}