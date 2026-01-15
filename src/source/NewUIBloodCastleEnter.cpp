// NewUIBloodCastleEnter.cpp: implementation of the CNewUIPartyInfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIBloodCastleEnter.h"
#include "NewUISystem.h"

#include "CharacterManager.h"
#include "DSPlaySound.h"

using namespace SEASON3B;

CNewUIEnterBloodCastle::CNewUIEnterBloodCastle()
{
    m_pNewUIMng = NULL;
    memset(&m_Pos, 0, sizeof(POINT));
    memset(&m_EnterUITextPos, 0, sizeof(POINT));

    m_iNumActiveBtn = 1;
    m_BtnEnterStartPos.x = m_BtnEnterStartPos.y = 0;
    m_dwBtnTextColor[0] = RGBA(150, 150, 150, 255);
    m_dwBtnTextColor[1] = RGBA(255, 255, 255, 255);

    m_iBloodCastleLimitLevel[0][0] = 15;  m_iBloodCastleLimitLevel[0][1] = 80;
    m_iBloodCastleLimitLevel[1][0] = 81;  m_iBloodCastleLimitLevel[1][1] = 130;
    m_iBloodCastleLimitLevel[2][0] = 131; m_iBloodCastleLimitLevel[2][1] = 180;
    m_iBloodCastleLimitLevel[3][0] = 181; m_iBloodCastleLimitLevel[3][1] = 230;
    m_iBloodCastleLimitLevel[4][0] = 231; m_iBloodCastleLimitLevel[4][1] = 280;
    m_iBloodCastleLimitLevel[5][0] = 281; m_iBloodCastleLimitLevel[5][1] = 330;
    m_iBloodCastleLimitLevel[6][0] = 331; m_iBloodCastleLimitLevel[6][1] = 400;
    m_iBloodCastleLimitLevel[7][0] = 0;   m_iBloodCastleLimitLevel[7][1] = 0;

    m_iBloodCastleLimitLevel[8][0] = 10;  m_iBloodCastleLimitLevel[8][1] = 60;
    m_iBloodCastleLimitLevel[9][0] = 61;  m_iBloodCastleLimitLevel[9][1] = 110;
    m_iBloodCastleLimitLevel[10][0] = 111; m_iBloodCastleLimitLevel[10][1] = 160;
    m_iBloodCastleLimitLevel[11][0] = 161; m_iBloodCastleLimitLevel[11][1] = 210;
    m_iBloodCastleLimitLevel[12][0] = 211; m_iBloodCastleLimitLevel[12][1] = 260;
    m_iBloodCastleLimitLevel[13][0] = 261; m_iBloodCastleLimitLevel[13][1] = 310;
    m_iBloodCastleLimitLevel[14][0] = 311; m_iBloodCastleLimitLevel[14][1] = 400;
    m_iBloodCastleLimitLevel[15][0] = 0;   m_iBloodCastleLimitLevel[15][1] = 0;
}

CNewUIEnterBloodCastle::~CNewUIEnterBloodCastle()
{
    Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIEnterBloodCastle::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_BLOODCASTLE, this);

    SetPos(x, y);

    LoadImages();

    // Exit Button
    m_BtnExit.ChangeButtonImgState(true, IMAGE_ENTERBC_BASE_WINDOW_BTN_EXIT, false);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnExit.ChangeToolTipText(GlobalText[1002], true);

    // Enter Button
    int iVal = 0;
    for (int i = 0; i < MAX_ENTER_GRADE; i++)
    {
        iVal = ENTER_BTN_VAL * i;
        m_BtnEnter[i].ChangeButtonImgState(true, IMAGE_ENTERBC_BASE_WINDOW_BTN_ENTER, true);
        m_BtnEnter[i].ChangeButtonInfo(m_BtnEnterStartPos.x, m_BtnEnterStartPos.y + iVal, 180, 29);
    }

    Show(false);

    return true;
}

//---------------------------------------------------------------------------------------------
// Release
void CNewUIEnterBloodCastle::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

//---------------------------------------------------------------------------------------------
// SetPos
void CNewUIEnterBloodCastle::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
    m_EnterUITextPos.x = m_Pos.x + 3;
    m_EnterUITextPos.y = m_Pos.y + 55;

    SetBtnPos(m_Pos.x + 6, m_Pos.y + 125);

    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);

    for (int i = 0; i < MAX_ENTER_GRADE; i++)
    {
        int iVal = ENTER_BTN_VAL * i;
        m_BtnEnter[i].ChangeButtonInfo(m_BtnEnterStartPos.x, m_BtnEnterStartPos.y + iVal, 180, 29);
    }
}

//---------------------------------------------------------------------------------------------
// SetBtnPos
void CNewUIEnterBloodCastle::SetBtnPos(int x, int y)
{
    m_BtnEnterStartPos.x = x;
    m_BtnEnterStartPos.y = y;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool CNewUIEnterBloodCastle::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, ENTERBC_BASE_WINDOW_WIDTH, ENTERBC_BASE_WINDOW_HEIGHT))
        return false;

    return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool CNewUIEnterBloodCastle::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_BLOODCASTLE);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

int CNewUIEnterBloodCastle::CheckLimitLV(int iIndex)
{
    int	iVal = 0;
    int iRet = 0;

    if (iIndex == 1)
    {
        iVal = 8;
    }

    int iLevel = CharacterAttribute->Level;

    if (gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == false)
    {
        for (int iCastleLV = 0; iCastleLV < MAX_ENTER_GRADE - 1; ++iCastleLV)
        {
            if (iLevel >= m_iBloodCastleLimitLevel[iVal + iCastleLV][0]
                && iLevel <= m_iBloodCastleLimitLevel[iVal + iCastleLV][1])
            {
                iRet = iCastleLV;
                break;
            }
        }
    }
    else
        iRet = MAX_ENTER_GRADE - 1;

    return iRet;
}

bool CNewUIEnterBloodCastle::Update()
{
    if (!IsVisible())
        return true;

    return true;
}

bool CNewUIEnterBloodCastle::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    // Base Window
    RenderImage(IMAGE_ENTERBC_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, float(ENTERBC_BASE_WINDOW_WIDTH), float(ENTERBC_BASE_WINDOW_HEIGHT));
    RenderImage(IMAGE_ENTERBC_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, float(ENTERBC_BASE_WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_ENTERBC_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(ENTERBC_BASE_WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_ENTERBC_BASE_WINDOW_RIGHT, m_Pos.x + float(ENTERBC_BASE_WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(ENTERBC_BASE_WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_ENTERBC_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(ENTERBC_BASE_WINDOW_HEIGHT) - 45.f, float(ENTERBC_BASE_WINDOW_WIDTH), 45.f);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(0xFFFFFFFF);
    g_pRenderText->SetBgColor(0x00000000);
    g_pRenderText->RenderText(m_Pos.x + 60, m_Pos.y + 12, GlobalText[846], 72, 0, RT3_SORT_CENTER);
    g_pRenderText->SetFont(g_hFont);

    wchar_t txtline[NUM_LINE_CMB][MAX_LENGTH_CMB] = { 0 };
    int tl = SeparateTextIntoLines(GlobalText[832], txtline[0], NUM_LINE_CMB, MAX_LENGTH_CMB);
    for (int j = 0; j < tl; ++j)
    {
        g_pRenderText->RenderText(m_EnterUITextPos.x, m_EnterUITextPos.y + j * 20, txtline[j], 190, 0, RT3_SORT_CENTER);
    }

    for (int i = 0; i < MAX_ENTER_GRADE; i++)
    {
        m_BtnEnter[i].Render();
    }

    m_BtnExit.Render();

    DisableAlphaBlend();

    return true;
}

bool CNewUIEnterBloodCastle::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_BLOODCASTLE);
        return true;
    }

    if (m_BtnExit.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_BLOODCASTLE);
        return true;
    }

    if ((m_iNumActiveBtn != -1) && (m_BtnEnter[m_iNumActiveBtn].UpdateMouseEvent() == true))
    {
        SocketClient->ToGameServer()->SendBloodCastleEnterRequest(m_iNumActiveBtn + 1, 0xFF);
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_BLOODCASTLE);
    }

    return false;
}

float CNewUIEnterBloodCastle::GetLayerDepth()
{
    return 4.1f;
}

void CNewUIEnterBloodCastle::OpenningProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    for (int i = 0; i < MAX_ENTER_GRADE; i++)
    {
        m_BtnEnter[i].ChangeTextColor(m_dwBtnTextColor[ENTERBTN_DISABLE]);
        m_BtnEnter[i].Lock();
    }

    int iLimitLVIndex = 0;
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD
        || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
    {
        iLimitLVIndex = 1;
    }

    m_iNumActiveBtn = CheckLimitLV(iLimitLVIndex);

    m_BtnEnter[m_iNumActiveBtn].UnLock();
    m_BtnEnter[m_iNumActiveBtn].ChangeTextColor(m_dwBtnTextColor[ENTERBTN_ENABLE]);

    wchar_t sztext[255] = { 0, };

    for (int i = 0; i < MAX_ENTER_GRADE - 1; i++)
    {
        swprintf(sztext, GlobalText[847], i + 1
            , m_iBloodCastleLimitLevel[(iLimitLVIndex * MAX_ENTER_GRADE) + i][0]
            , m_iBloodCastleLimitLevel[(iLimitLVIndex * MAX_ENTER_GRADE) + i][1]);
        m_BtnEnter[i].SetFont(g_hFontBold);
        m_BtnEnter[i].ChangeText(sztext);
    }

    swprintf(sztext, GlobalText[1779], 8);

    m_BtnEnter[MAX_ENTER_GRADE - 1].SetFont(g_hFontBold);
    m_BtnEnter[MAX_ENTER_GRADE - 1].ChangeText(sztext);
}

void CNewUIEnterBloodCastle::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

void CNewUIEnterBloodCastle::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_ENTERBC_BASE_WINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_ENTERBC_BASE_WINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_ENTERBC_BASE_WINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_ENTERBC_BASE_WINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_ENTERBC_BASE_WINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_ENTERBC_BASE_WINDOW_BTN_EXIT, GL_LINEAR);				// Exit Button
    LoadBitmap(L"Interface\\newui_btn_empty_big.tga", IMAGE_ENTERBC_BASE_WINDOW_BTN_ENTER, GL_LINEAR);		// Enter Button
}

void CNewUIEnterBloodCastle::UnloadImages()
{
    DeleteBitmap(IMAGE_ENTERBC_BASE_WINDOW_BACK);
    DeleteBitmap(IMAGE_ENTERBC_BASE_WINDOW_TOP);
    DeleteBitmap(IMAGE_ENTERBC_BASE_WINDOW_LEFT);
    DeleteBitmap(IMAGE_ENTERBC_BASE_WINDOW_RIGHT);
    DeleteBitmap(IMAGE_ENTERBC_BASE_WINDOW_BOTTOM);
    DeleteBitmap(IMAGE_ENTERBC_BASE_WINDOW_BTN_EXIT);		// Exit Button
    DeleteBitmap(IMAGE_ENTERBC_BASE_WINDOW_BTN_ENTER);	// Enter Button
}