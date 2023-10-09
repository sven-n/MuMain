// NewUICatapultWindow.cpp: implementation of the CNewUICatapultWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUICatapultWindow.h"
#include "NewUISystem.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"


using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUICatapultWindow::CCatapultGroupButton::CCatapultGroupButton()
{
    m_pButton = NULL;

    Initialize();
}

SEASON3B::CNewUICatapultWindow::CCatapultGroupButton::~CCatapultGroupButton()
{
    SAFE_DELETE_ARRAY(m_pButton);
}

void SEASON3B::CNewUICatapultWindow::CCatapultGroupButton::Initialize()
{
    SAFE_DELETE_ARRAY(m_pButton);

    m_iType = 0;
    m_iIndex = -1;
}

void SEASON3B::CNewUICatapultWindow::CCatapultGroupButton::Create(int iType, POINT ptWindow)
{
    Initialize();

    m_iType = iType;

    if (iType == CATAPULT_ATTACK)
    {
        m_iBtnNum = 4;
        m_pButton = new CNewUIButton[m_iBtnNum];
        m_pButton[0].ChangeText(GlobalText[1402]);
        m_pButton[0].ChangeTextBackColor(RGBA(255, 255, 255, 0));
        m_pButton[0].ChangeButtonImgState(true, IMAGE_CATAPULT_BTN_SMALL, true);
        m_pButton[0].ChangeButtonInfo(ptWindow.x + 22, ptWindow.y + 135, 46, 36);
        m_pButton[0].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_pButton[0].ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
        m_pButton[1].ChangeText(GlobalText[1403]);
        m_pButton[1].ChangeTextBackColor(RGBA(255, 255, 255, 0));
        m_pButton[1].ChangeButtonImgState(true, IMAGE_CATAPULT_BTN_SMALL, true);
        m_pButton[1].ChangeButtonInfo(ptWindow.x + 74, ptWindow.y + 135, 46, 36);
        m_pButton[1].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_pButton[1].ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
        m_pButton[2].ChangeText(GlobalText[1404]);
        m_pButton[2].ChangeTextBackColor(RGBA(255, 255, 255, 0));
        m_pButton[2].ChangeButtonImgState(true, IMAGE_CATAPULT_BTN_SMALL, true);
        m_pButton[2].ChangeButtonInfo(ptWindow.x + 126, ptWindow.y + 135, 46, 36);
        m_pButton[2].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_pButton[2].ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
        m_pButton[3].ChangeText(GlobalText[1405]);
        m_pButton[3].ChangeTextBackColor(RGBA(255, 255, 255, 0));
        m_pButton[3].ChangeButtonImgState(true, IMAGE_CATAPULT_BTN_BIG, true);
        m_pButton[3].ChangeButtonInfo(ptWindow.x + 59, ptWindow.y + 182, 77, 47);
        m_pButton[3].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_pButton[3].ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
    }
    else if (iType == CATAPULT_DEFENSE)
    {
        m_iBtnNum = 3;
        m_pButton = new CNewUIButton[m_iBtnNum];
        m_pButton[0].ChangeText(GlobalText[1406]);
        m_pButton[0].ChangeTextBackColor(RGBA(255, 255, 255, 0));
        m_pButton[0].ChangeButtonImgState(true, IMAGE_CATAPULT_BTN_BIG, true);
        m_pButton[0].ChangeButtonInfo(ptWindow.x + 18, ptWindow.y + 125, 77, 47);
        m_pButton[0].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_pButton[0].ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
        m_pButton[1].ChangeText(GlobalText[1407]);
        m_pButton[1].ChangeTextBackColor(RGBA(255, 255, 255, 0));
        m_pButton[1].ChangeButtonImgState(true, IMAGE_CATAPULT_BTN_BIG, true);
        m_pButton[1].ChangeButtonInfo(ptWindow.x + 97, ptWindow.y + 125, 77, 47);
        m_pButton[1].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_pButton[1].ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
        m_pButton[2].ChangeText(GlobalText[1408]);
        m_pButton[2].ChangeTextBackColor(RGBA(255, 255, 255, 0));
        m_pButton[2].ChangeButtonImgState(true, IMAGE_CATAPULT_BTN_BIG, true);
        m_pButton[2].ChangeButtonInfo(ptWindow.x + 56, ptWindow.y + 179, 77, 47);
        m_pButton[2].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_pButton[2].ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
    }
}

void SEASON3B::CNewUICatapultWindow::CCatapultGroupButton::AllUnLock()
{
    for (int i = 0; i < m_iBtnNum; ++i)
    {
        m_pButton[i].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_pButton[i].ChangeTextColor(RGBA(255, 255, 255, 255));
        m_pButton[i].UnLock();
    }
}

void SEASON3B::CNewUICatapultWindow::CCatapultGroupButton::BtnSelected(int iIndex)
{
    if (iIndex < 0 || iIndex > m_iBtnNum)
    {
        return;
    }

    AllUnLock();

    m_pButton[iIndex].ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
    m_pButton[iIndex].ChangeTextColor(RGBA(100, 100, 100, 255));
    m_pButton[iIndex].Lock();
}

int SEASON3B::CNewUICatapultWindow::CCatapultGroupButton::GetIndex()
{
    return m_iIndex;
}

int SEASON3B::CNewUICatapultWindow::CCatapultGroupButton::UpdateMouseEvent()
{
    int iResult = -1;

    for (int i = 0; i < m_iBtnNum; ++i)
    {
        if (m_pButton[i].UpdateMouseEvent() == true)
        {
            BtnSelected(i);
            m_iIndex = i;
            iResult = i;
            break;
        }
    }

    return iResult;
}

void SEASON3B::CNewUICatapultWindow::CCatapultGroupButton::Render()
{
    int iCount = 0;

    if (m_iType == CATAPULT_ATTACK)
    {
        iCount = 4;
    }
    else if (m_iType == CATAPULT_DEFENSE)
    {
        iCount = 3;
    }

    for (int i = 0; i < iCount; ++i)
    {
        m_pButton[i].Render();
    }
}

SEASON3B::CNewUICatapultWindow::CNewUICatapultWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;

    OpenningProcess();
}

SEASON3B::CNewUICatapultWindow::~CNewUICatapultWindow()
{
    Release();
}

bool SEASON3B::CNewUICatapultWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CATAPULT, this);

    SetPos(x, y);

    LoadImages();

    SetButtonInfo();

    Show(false);

    return true;
}

void SEASON3B::CNewUICatapultWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUICatapultWindow::SetButtonInfo()
{
    m_BtnExit.ChangeButtonImgState(true, IMAGE_CATAPULT_BTN_EXIT, false);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnExit.ChangeToolTipText(GlobalText[1002], true);
    m_BtnFire.ChangeText(GlobalText[1499]);
    m_BtnFire.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnFire.ChangeButtonImgState(true, IMAGE_CATAPULT_BTN_FIRE, true);
    m_BtnFire.ChangeButtonInfo(m_Pos.x + 41, m_Pos.y + 250, 108, 29);
    m_BtnFire.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnFire.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void SEASON3B::CNewUICatapultWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnFire.ChangeButtonInfo(m_Pos.x + 41, m_Pos.y + 250, 108, 29);
}

bool SEASON3B::CNewUICatapultWindow::UpdateMouseEvent()
{
    if (BtnProcess() == true)
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUICatapultWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CATAPULT) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_CATAPULT);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

bool SEASON3B::CNewUICatapultWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUICatapultWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderOutlineUpper(m_Pos.x + 0, m_Pos.y + 120, 162, 100);
    RenderOutlineLower(m_Pos.x + 0, m_Pos.y + 120, 162, 100);
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUICatapultWindow::RenderFrame()
{
    RenderImage(IMAGE_CATAPULT_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_CATAPULT_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_CATAPULT_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_CATAPULT_RIGHT, m_Pos.x + 190 - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_CATAPULT_BOTTOM, m_Pos.x, m_Pos.y + 429 - 45, 190.f, 45.f);
}

void SEASON3B::CNewUICatapultWindow::RenderTexts()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetBgColor(0);
    if (m_iType == CATAPULT_ATTACK)
    {
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13.f, GlobalText[1400], 190, 0, RT3_SORT_CENTER);
    }
    else if (m_iType == CATAPULT_DEFENSE)
    {
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13.f, GlobalText[1401], 190, 0, RT3_SORT_CENTER);
    }

    float fLine = 50.f;
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + fLine, GlobalText[1409], 190, 0, RT3_SORT_CENTER);
    fLine += 15.f;
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + fLine, GlobalText[1410], 190, 0, RT3_SORT_CENTER);
    fLine += 15.f;
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + fLine, GlobalText[1411], 190, 0, RT3_SORT_CENTER);
}

void SEASON3B::CNewUICatapultWindow::RenderButtons()
{
    m_BtnExit.Render();
    m_BtnFire.Render();

    m_BtnChoiceArea.Render();
}

void SEASON3B::CNewUICatapultWindow::RenderOutlineUpper(float fPos_x, float fPos_y, float fWidth, float fHeight)
{
    POINT ptOrigin = { (long)fPos_x, (long)fPos_y };
    float fBoxWidth = fWidth;

    RenderImage(IMAGE_CATAPULT_TABLE_TOP_LEFT, ptOrigin.x + 12, ptOrigin.y - 4, 14, 14);
    RenderImage(IMAGE_CATAPULT_TABLE_TOP_RIGHT, ptOrigin.x + fBoxWidth + 4, ptOrigin.y - 4, 14, 14);
    RenderImage(IMAGE_CATAPULT_TABLE_TOP_PIXEL, ptOrigin.x + 25, ptOrigin.y - 4, fBoxWidth - 21, 14);
}

void SEASON3B::CNewUICatapultWindow::RenderOutlineLower(float fPos_x, float fPos_y, float fWidth, float fHeight)
{
    POINT ptOrigin = { (long)fPos_x, (long)fPos_y };
    float fBoxWidth = fWidth;
    float fBoxHeight = fHeight;

    RenderImage(IMAGE_CATAPULT_TABLE_LEFT_PIXEL, ptOrigin.x + 12, ptOrigin.y + 9, 14, fBoxHeight);
    RenderImage(IMAGE_CATAPULT_TABLE_RIGHT_PIXEL, ptOrigin.x + fBoxWidth + 4, ptOrigin.y + 9, 14, fBoxHeight);
    RenderImage(IMAGE_CATAPULT_TABLE_BOTTOM_LEFT, ptOrigin.x + 12, ptOrigin.y + fBoxHeight + 3, 14, 14);
    RenderImage(IMAGE_CATAPULT_TABLE_BOTTOM_RIGHT, ptOrigin.x + fBoxWidth + 4, ptOrigin.y + fBoxHeight + 3, 14, 14);
    RenderImage(IMAGE_CATAPULT_TABLE_BOTTOM_PIXEL, ptOrigin.x + 25, ptOrigin.y + fBoxHeight + 3, fBoxWidth - 21, 14);
}

float SEASON3B::CNewUICatapultWindow::GetLayerDepth()
{
    return 5.0f;
}

void SEASON3B::CNewUICatapultWindow::OpenningProcess()
{
    m_iType = 0;
    m_iNpcKey = 0;
    Vector(0.f, 0.f, 0.f, m_vCameraPos);

    m_BtnFire.Lock();
    m_BtnFire.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
    m_BtnFire.ChangeTextColor(RGBA(100, 100, 100, 255));
}

void SEASON3B::CNewUICatapultWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

void SEASON3B::CNewUICatapultWindow::Init(int iKey, int iType)
{
    m_iNpcKey = iKey;
    m_iType = iType;

    if (m_iType == CATAPULT_ATTACK)
    {
        m_BtnChoiceArea.Create(CATAPULT_ATTACK, m_Pos);
    }
    else if (m_iType == CATAPULT_DEFENSE)
    {
        m_BtnChoiceArea.Create(CATAPULT_DEFENSE, m_Pos);
    }
}

void SEASON3B::CNewUICatapultWindow::DoFire(int iKey, int iResult, int iType, int iPositionX, int iPositionY)
{
    int iIndex = FindCharacterIndex(iKey);
    CHARACTER* c = &CharactersClient[iIndex];
    OBJECT* o = &c->Object;

    SetAction(o, 1);

    BYTE bySubType = 0;
    BYTE byKeyH = 0;
    BYTE byKeyL = 0;
    if (iResult == 1)
    {
        bySubType = 1;
        byKeyH = (BYTE)(iKey & 0xff);
        byKeyL = (BYTE)(iKey >> 8);
    }
    else if (iResult == 2)
    {
        bySubType = 0;
    }

    vec3_t vPos, vTargetPos;
    Vector(o->Position[0], o->Position[1], 500.f, vPos);
    Vector(iPositionX * TERRAIN_SCALE, iPositionY * TERRAIN_SCALE, 100.f, vTargetPos);
    switch (iType)
    {
    case 1:
        CreateEffect(MODEL_FLY_BIG_STONE1, vPos, vTargetPos, o->Light, bySubType, &Hero->Object, 1, byKeyH, byKeyL);
        break;

    case 2:
        CreateEffect(MODEL_FLY_BIG_STONE2, vPos, vTargetPos, o->Light, bySubType, &Hero->Object, 1, byKeyH, byKeyL);
        break;
    }

    PlayBuffer(SOUND_BC_CATAPULT_ATTACK);
}

void SEASON3B::CNewUICatapultWindow::DoFireFixStartPosition(int iType, int iPositionX, int iPositionY)
{
    vec3_t vPos, vTargetPos;

    Vector(iPositionX * TERRAIN_SCALE, iPositionY * TERRAIN_SCALE, 100.f, vTargetPos);

    switch (iType)
    {
    case 1:
        Vector(9200, 3000, 500.f, vPos);
        CreateEffect(MODEL_FLY_BIG_STONE1, vPos, vTargetPos, Hero->Object.Light, 0, &Hero->Object, 1);
        break;

    case 2:
        Vector(9400, 19000, 500.f, vPos);
        CreateEffect(MODEL_FLY_BIG_STONE2, vPos, vTargetPos, Hero->Object.Light, 0, &Hero->Object, 1);
        break;
    }

    vec3_t vLight = { 1.f, 0.3f, 0.1f };
    CreateEffect(BITMAP_SHOCK_WAVE, vTargetPos, Hero->Object.Angle, vLight, 6);

    PlayBuffer(SOUND_BC_CATAPULT_ATTACK);
}

void SEASON3B::CNewUICatapultWindow::SetCameraPos(float x, float y, float z)
{
    Vector(x, y, z, m_vCameraPos);
}

void SEASON3B::CNewUICatapultWindow::GetCameraPos(vec3_t& vPos)
{
    if (m_vCameraPos[0] != 0.f || m_vCameraPos[1] != 0.f || m_vCameraPos[2] != 0.f)
    {
        VectorCopy(m_vCameraPos, vPos);
    }
    else
    {
        VectorCopy(Hero->Object.Position, vPos);
    }
}

void SEASON3B::CNewUICatapultWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_CATAPULT_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_CATAPULT_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_CATAPULT_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_CATAPULT_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_CATAPULT_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_CATAPULT_BTN_EXIT, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_CATAPULT_BTN_FIRE, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_Btn_gate.tga", IMAGE_CATAPULT_BTN_SMALL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Btn_round.tga", IMAGE_CATAPULT_BTN_BIG, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_item_table01(L).tga", IMAGE_CATAPULT_TABLE_TOP_LEFT);
    LoadBitmap(L"Interface\\newui_item_table01(R).tga", IMAGE_CATAPULT_TABLE_TOP_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table02(L).tga", IMAGE_CATAPULT_TABLE_BOTTOM_LEFT);
    LoadBitmap(L"Interface\\newui_item_table02(R).tga", IMAGE_CATAPULT_TABLE_BOTTOM_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table03(Up).tga", IMAGE_CATAPULT_TABLE_TOP_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(Dw).tga", IMAGE_CATAPULT_TABLE_BOTTOM_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(L).tga", IMAGE_CATAPULT_TABLE_LEFT_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(R).tga", IMAGE_CATAPULT_TABLE_RIGHT_PIXEL);
}

void SEASON3B::CNewUICatapultWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_CATAPULT_BTN_BIG);
    DeleteBitmap(IMAGE_CATAPULT_BTN_SMALL);
    DeleteBitmap(IMAGE_CATAPULT_BTN_FIRE);

    DeleteBitmap(IMAGE_CATAPULT_TABLE_RIGHT_PIXEL);
    DeleteBitmap(IMAGE_CATAPULT_TABLE_LEFT_PIXEL);
    DeleteBitmap(IMAGE_CATAPULT_TABLE_BOTTOM_PIXEL);
    DeleteBitmap(IMAGE_CATAPULT_TABLE_TOP_PIXEL);
    DeleteBitmap(IMAGE_CATAPULT_TABLE_BOTTOM_RIGHT);
    DeleteBitmap(IMAGE_CATAPULT_TABLE_BOTTOM_LEFT);
    DeleteBitmap(IMAGE_CATAPULT_TABLE_TOP_RIGHT);
    DeleteBitmap(IMAGE_CATAPULT_TABLE_TOP_LEFT);

    DeleteBitmap(IMAGE_CATAPULT_BTN_EXIT);
    DeleteBitmap(IMAGE_CATAPULT_BOTTOM);
    DeleteBitmap(IMAGE_CATAPULT_RIGHT);
    DeleteBitmap(IMAGE_CATAPULT_LEFT);
    DeleteBitmap(IMAGE_CATAPULT_TOP);
    DeleteBitmap(IMAGE_CATAPULT_BACK);
}

bool SEASON3B::CNewUICatapultWindow::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_CATAPULT);
        return true;
    }

    if (m_BtnExit.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_CATAPULT);
        return true;
    }

    int iIndex = 0;
    iIndex = m_BtnChoiceArea.UpdateMouseEvent();

    if (iIndex > -1)
    {
        m_BtnFire.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_BtnFire.ChangeTextColor(RGBA(255, 255, 255, 255));
        m_BtnFire.UnLock();
        return true;
    }

    iIndex = m_BtnChoiceArea.GetIndex();
    if (iIndex > -1 && m_BtnFire.UpdateMouseEvent() == true)
    {
        SocketClient->ToGameServer()->SendFireCatapultRequest(m_iNpcKey, iIndex + 1);
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_CATAPULT);
    }

    return false;
}