//*****************************************************************************
// Desc: implementation of the CServerSelWin class.
//*****************************************************************************

#include "stdafx.h"
#include "ServerSelWin.h"
#include "Input.h"
#include "UIMng.h"
#include "local.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

#include "UIControls.h"

#include "NewUISystem.h"
#include "ServerListManager.h"

#define	SSW_GAP_WIDTH	28
#define	SSW_GAP_HEIGHT	5
#define	SSW_GB_POS_X	16
#define	SSW_GB_POS_Y	19




using namespace SEASON3A;

CServerSelWin::CServerSelWin()
{
}

CServerSelWin::~CServerSelWin()
{
}

void CServerSelWin::Create()
{
    CWin::Create(0, 0, -2);

    m_iSelectServerBtnIndex = -1;

    int i;

    for (i = 0; i < SSW_SERVER_G_MAX; ++i)
    {
        m_aServerGroupBtn[i].Create(SERVER_GROUP_BTN_WIDTH, SERVER_GROUP_BTN_HEIGHT, BITMAP_LOG_IN, 4, 2, 1, -1, 3);
        CWin::RegisterButton(&m_aServerGroupBtn[i]);
    }

    for (i = 0; i < SSW_SERVER_MAX; ++i)
    {
        m_aServerBtn[i].Create(SERVER_BTN_WIDTH, SERVER_BTN_HEIGHT, BITMAP_LOG_IN + 1, 3, 2, 1);
        CWin::RegisterButton(&m_aServerBtn[i]);
        m_aServerGauge[i].Create(160, 4, BITMAP_LOG_IN + 2);
    }

    SImgInfo aiiDeco[2] =
    {
        { BITMAP_LOG_IN + 3, 0, 0, 68, 95 },
        { BITMAP_LOG_IN + 3, 68, 0, 68, 95 }
    };
    m_aBtnDeco[0].Create(&aiiDeco[0], 8, 19);
    m_aBtnDeco[1].Create(&aiiDeco[1], 60, 19);

    SImgInfo aiiArrow[2] =
    {
        { BITMAP_LOG_IN + 3, 136, 0, 23, 29 },
        { BITMAP_LOG_IN + 3, 136, 30, 23, 29 }
    };
    m_aArrowDeco[0].Create(&aiiArrow[0], 1, 2);
    m_aArrowDeco[1].Create(&aiiArrow[1], 23, 2);

    SImgInfo aiiDescBg[WE_BG_MAX] =
    {
        { BITMAP_LOG_IN + 11, 0, 0, 4, 4 },
        { BITMAP_LOG_IN + 12, 0, 0, 512, 6 },
        { BITMAP_LOG_IN + 12, 0, 6, 512, 6 },
        { BITMAP_LOG_IN + 13, 0, 0, 3, 4 },
        { BITMAP_LOG_IN + 13, 3, 0, 3, 4 }
    };
    m_winDescription.Create(aiiDescBg, 1, 10);
    m_winDescription.SetLine(10);

    CWin::SetSize((SERVER_GROUP_BTN_WIDTH + SSW_GAP_WIDTH) * 2 + SERVER_BTN_WIDTH, SERVER_BTN_HEIGHT * SSW_SERVER_MAX + SSW_GAP_HEIGHT * 2 + SERVER_GROUP_BTN_HEIGHT + m_winDescription.GetHeight());
}

void CServerSelWin::PreRelease()
{
    int i;

    for (i = 0; i < SSW_SERVER_MAX; ++i)
    {
        m_aServerGauge[i].Release();
    }

    for (i = 0; i < 2; ++i)
    {
        m_aBtnDeco[i].Release();
        m_aArrowDeco[i].Release();
    }

    m_winDescription.Release();
}

void CServerSelWin::SetPosition(int nXCoord, int nYCoord)
{
    CWin::SetPosition(nXCoord, nYCoord);

    int nServerGBtnWidth = m_aServerGroupBtn[0].GetWidth();
    int nServerGBtnHeight = m_aServerGroupBtn[0].GetHeight();
    int nServerBtnWidth = m_aServerBtn[0].GetWidth();
    int nServerBtnHeight = m_aServerBtn[0].GetHeight();
    int nDescGgHeight = m_winDescription.GetHeight();
    int nBtnPosY;
    int i;

    int nServerGBtnBasePosY = nYCoord + CWin::GetHeight() - (nServerGBtnHeight * 11 + SSW_GAP_HEIGHT * 2 + nDescGgHeight);
    int nRServerGBtnPosX = nXCoord + nServerGBtnWidth + nServerBtnWidth + (SSW_GAP_WIDTH * 2);

    int icntServreGroup = 0;
    m_aServerGroupBtn[icntServreGroup++].SetPosition(nXCoord + (CWin::GetWidth() - nServerGBtnWidth) / 2, nYCoord + CWin::GetHeight() - nServerGBtnHeight - SSW_GAP_HEIGHT - nDescGgHeight);

    for (i = 0; i < SSW_LEFT_SERVER_G_MAX; i++)
    {
        nBtnPosY = nServerGBtnBasePosY + nServerGBtnHeight * i;
        m_aServerGroupBtn[icntServreGroup++].SetPosition(nXCoord, nBtnPosY);
    }

    for (i = 0; i < SSW_RIGHT_SERVER_G_MAX; i++)
    {
        nBtnPosY = nServerGBtnBasePosY + nServerGBtnHeight * i;
        m_aServerGroupBtn[icntServreGroup++].SetPosition(nRServerGBtnPosX, nBtnPosY);
    }

    m_winDescription.SetPosition(nXCoord - ((m_winDescription.GetWidth() - CWin::GetWidth()) / 2), nYCoord + CWin::GetHeight() - m_winDescription.GetHeight());

    m_aBtnDeco[0].SetPosition(m_aServerGroupBtn[1].GetXPos(), m_aServerGroupBtn[1].GetYPos());
    m_aBtnDeco[1].SetPosition(m_aServerGroupBtn[SSW_LEFT_SERVER_G_MAX + 1].GetXPos() + SERVER_GROUP_BTN_WIDTH, m_aServerGroupBtn[SSW_LEFT_SERVER_G_MAX + 1].GetYPos());

    int a = m_aServerGroupBtn[1].GetXPos();
}

void CServerSelWin::SetServerBtnPosition()
{
    if (m_iSelectServerBtnIndex == -1)
        return;

    int nServerBtnPosX = m_aServerGroupBtn[1].GetXPos() + m_aServerGroupBtn[0].GetWidth() + SSW_GAP_WIDTH;

    int nServerBtnHeight = m_aServerBtn[0].GetHeight();

    int nLServerGBtnHeightSum = m_aServerGroupBtn[1].GetHeight() * 10;

    int nServerBtnHeightSum = nServerBtnHeight * m_icntServer;

    int nLServerGBtnTop = m_aServerGroupBtn[1].GetYPos();

    int nServerBtnBasePosY = nLServerGBtnHeightSum > nServerBtnHeightSum ? nLServerGBtnTop : nLServerGBtnTop - (nServerBtnHeightSum - nLServerGBtnHeightSum);

    for (int i = 0; i < m_pSelectServerGroup->GetServerSize(); i++)
    {
        m_aServerBtn[i].SetPosition(nServerBtnPosX, nServerBtnBasePosY + nServerBtnHeight * i);
        m_aServerGauge[i].SetPosition(m_aServerBtn[i].GetXPos() + SSW_GB_POS_X, m_aServerBtn[i].GetYPos() + SSW_GB_POS_Y);
    }
}

void CServerSelWin::SetArrowSpritePosition()
{
    if (m_iSelectServerBtnIndex == -1)
        return;

    if ((m_iSelectServerBtnIndex >= 0) && (m_iSelectServerBtnIndex <= SSW_LEFT_SERVER_G_MAX))
    {
        m_aArrowDeco[0].SetPosition(m_aServerGroupBtn[m_iSelectServerBtnIndex].GetXPos() + SERVER_GROUP_BTN_WIDTH, m_aServerGroupBtn[m_iSelectServerBtnIndex].GetYPos());
    }
    else if ((m_iSelectServerBtnIndex > SSW_LEFT_SERVER_G_MAX) && (m_iSelectServerBtnIndex < SSW_SERVER_G_MAX))
    {
        m_aArrowDeco[1].SetPosition(m_aServerGroupBtn[m_iSelectServerBtnIndex].GetXPos(), m_aServerGroupBtn[m_iSelectServerBtnIndex].GetYPos());
    }
}

void CServerSelWin::UpdateDisplay()
{
    m_pSelectServerGroup = NULL;
    m_icntServerGroup = 0;
    m_icntServer = 0;
    m_icntLeftServerGroup = 0;
    m_icntRightServerGroup = 0;
    m_bTestServerBtn = false;

    DWORD adwServerGBtnClr[BTN_IMG_MAX] =
    {
        CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0,
        CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0
    };

    DWORD adwServerBtnClr[4][4] =
    {
        { CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0 },
        { CLRDW_YELLOW, CLRDW_YELLOW, CLRDW_BR_YELLOW, 0 },
        { CLRDW_ORANGE, CLRDW_ORANGE, CLRDW_BR_ORANGE, 0 },
        { CLRDW_ORANGE, CLRDW_ORANGE, CLRDW_BR_ORANGE, 0 },
    };

    m_icntServerGroup = g_ServerListManager->GetServerGroupSize();

    if (m_icntServerGroup < 1)
        return;

    CServerGroup* pServerGroup = NULL;

    g_ServerListManager->SetFirst();

    while (g_ServerListManager->GetNext(pServerGroup))
    {
        if (pServerGroup->m_iWidthPos == CServerGroup::SBP_CENTER)
        {
            if (m_bTestServerBtn == true)
                continue;

            m_aServerGroupBtn[0].SetText(pServerGroup->m_szName, adwServerGBtnClr);
            pServerGroup->m_iBtnPos = 0;
            m_bTestServerBtn = true;
        }
        else if (pServerGroup->m_iWidthPos == CServerGroup::SBP_LEFT)
        {
            if (m_icntLeftServerGroup >= SSW_LEFT_SERVER_G_MAX)
                continue;

            m_aServerGroupBtn[m_icntLeftServerGroup + 1].SetText(pServerGroup->m_szName, adwServerGBtnClr);
            pServerGroup->m_iBtnPos = m_icntLeftServerGroup + 1;

            m_icntLeftServerGroup++;
        }
        else if (pServerGroup->m_iWidthPos == CServerGroup::SBP_RIGHT)
        {
            if (m_icntRightServerGroup >= SSW_RIGHT_SERVER_G_MAX)
                continue;

            m_aServerGroupBtn[SSW_LEFT_SERVER_G_MAX + m_icntRightServerGroup + 1].SetText(pServerGroup->m_szName, adwServerGBtnClr);
            pServerGroup->m_iBtnPos = SSW_LEFT_SERVER_G_MAX + m_icntRightServerGroup + 1;

            m_icntRightServerGroup++;
        }
    }

    ShowServerGBtns();
    ShowDecoSprite();

    memset(m_szDescription, 0, sizeof(char) * SSW_DESC_LINE_MAX * SSW_DESC_ROW_MAX);

    if (m_iSelectServerBtnIndex != -1)
    {
        m_pSelectServerGroup = g_ServerListManager->GetServerGroupByBtnPos(m_iSelectServerBtnIndex);
    }

    if (m_pSelectServerGroup == NULL)
        return;

    m_icntServer = m_pSelectServerGroup->GetServerSize();

    if (m_icntServer < 1)
        return;

    CServerInfo* pServerInfo = NULL;

    m_pSelectServerGroup->SetFirst();

    int icntServer = 0;
    while (m_pSelectServerGroup->GetNext(pServerInfo))
    {
        m_aServerBtn[icntServer].SetText(pServerInfo->m_bName, adwServerBtnClr[pServerInfo->m_byNonPvP]);
        m_aServerGauge[icntServer].SetValue(pServerInfo->m_iPercent, 100);
        icntServer++;
    }

    ::SeparateTextIntoLines(m_pSelectServerGroup->m_szDescription, m_szDescription[0], SSW_DESC_LINE_MAX, SSW_DESC_ROW_MAX);

    SetArrowSpritePosition();
    SetServerBtnPosition();
    ShowArrowSprite();
    ShowServerBtns();
}

void CServerSelWin::Show(bool bShow)
{
    CWin::Show(bShow);
}

void CServerSelWin::ShowServerGBtns()
{
    int i;

    if (m_bTestServerBtn == true)
    {
        m_aServerGroupBtn[0].Show(CWin::m_bShow);
    }
    else
    {
        m_aServerGroupBtn[0].Show(false);
    }

    for (i = 1; i < m_icntLeftServerGroup + 1; i++)
    {
        m_aServerGroupBtn[i].Show(CWin::m_bShow);
    }
    for (; i < SSW_LEFT_SERVER_G_MAX; ++i)
    {
        m_aServerGroupBtn[i].Show(false);
    }

    for (i = SSW_LEFT_SERVER_G_MAX + 1; i < SSW_RIGHT_SERVER_G_MAX + 1 + m_icntRightServerGroup; i++)
    {
        m_aServerGroupBtn[i].Show(CWin::m_bShow);
    }
    for (; i < SSW_SERVER_G_MAX; i++)
    {
        m_aServerGroupBtn[i].Show(false);
    }
}

void CServerSelWin::ShowDecoSprite()
{
    if (m_icntLeftServerGroup > 0)
    {
        m_aBtnDeco[0].Show(CWin::m_bShow);
    }
    else
    {
        m_aBtnDeco[0].Show(false);
    }

    if (m_icntRightServerGroup > 0)
    {
        m_aBtnDeco[1].Show(CWin::m_bShow);
    }
    else
    {
        m_aBtnDeco[1].Show(false);
    }
}

void CServerSelWin::ShowArrowSprite()
{
    if ((m_iSelectServerBtnIndex >= 0) && (m_iSelectServerBtnIndex <= SSW_LEFT_SERVER_G_MAX))
    {
        m_aArrowDeco[0].Show(CWin::m_bShow);
        m_aArrowDeco[1].Show(false);
    }
    else if ((m_iSelectServerBtnIndex > SSW_LEFT_SERVER_G_MAX) && (m_iSelectServerBtnIndex < SSW_SERVER_G_MAX))
    {
        m_aArrowDeco[0].Show(false);
        m_aArrowDeco[1].Show(CWin::m_bShow);
    }
    else
    {
        m_aArrowDeco[0].Show(false);
        m_aArrowDeco[1].Show(false);
    }
}

void CServerSelWin::ShowServerBtns()
{
    if (m_iSelectServerBtnIndex == -1)
    {
        m_winDescription.Show(false);
        return;
    }

    int i;
    for (i = 0; i < m_icntServer; i++)
    {
        m_aServerBtn[i].Show(CWin::m_bShow);
        m_aServerGauge[i].Show(CWin::m_bShow);
    }
    for (; i < SSW_SERVER_MAX; i++)
    {
        m_aServerBtn[i].Show(false);
        m_aServerGauge[i].Show(false);
    }

    m_winDescription.Show(CWin::m_bShow);
}

bool CServerSelWin::CursorInWin(int nArea)
{
    if (!CWin::m_bShow)
        return false;

    switch (nArea)
    {
    case WA_MOVE:
        return false;
    }

    return CWin::CursorInWin(nArea);
}

void CServerSelWin::UpdateWhileActive(double dDeltaTick)
{
    int i;

    for (i = 0; i < SSW_SERVER_G_MAX; i++)
    {
        if (m_aServerGroupBtn[i].IsClick())
        {
            if (m_iSelectServerBtnIndex != -1)
            {
                m_aServerGroupBtn[m_iSelectServerBtnIndex].SetCheck(false);
            }

            m_aServerGroupBtn[i].SetCheck(true);
            m_iSelectServerBtnIndex = i;

            SocketClient->ToConnectServer()->SendServerListRequest();
        }
    }

    if (m_pSelectServerGroup == NULL)
        return;

    CServerInfo* pServerInfo = NULL;
    for (i = 0; i < m_icntServer; i++)
    {
        if (m_aServerBtn[i].IsClick())
        {
            pServerInfo = m_pSelectServerGroup->GetServerInfo(i);

            if (pServerInfo == NULL)
                return;

            if (pServerInfo->m_iPercent < 100)
            {
                CUIMng::Instance().HideWin(this);

                SocketClient->ToConnectServer()->SendConnectionInfoRequest(static_cast<uint16_t>(pServerInfo->m_iConnectIndex));
                g_pSystemLogBox->AddText(GlobalText[470], SEASON3B::TYPE_SYSTEM_MESSAGE);
                g_pSystemLogBox->AddText(GlobalText[471], SEASON3B::TYPE_SYSTEM_MESSAGE);

                //if (m_pSelectServerGroup->m_iSequence == 0)
                //{
                //    bTestServer = true;
                //}

                g_ServerListManager->SetSelectServerInfo(m_pSelectServerGroup->m_szName, pServerInfo->m_iIndex, pServerInfo->m_byNonPvP);

                break;
            }
            else if (pServerInfo->m_iPercent < 128)
            {
                CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_BUSY);
            }
        }
    }
}

void CServerSelWin::RenderControls()
{
    int i = 0;

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->SetBgColor(0);

    CWin::RenderButtons();

    if (m_pSelectServerGroup != NULL)
    {
        for (i = 0; i < m_icntServer; i++)
        {
            m_aServerGauge[i].Render();
        }

        if (m_pSelectServerGroup->m_bPvPServer == true)
        {
            g_pRenderText->SetTextColor(ARGB(255, 255, 255, 255));
            g_pRenderText->RenderText(90, 164 - 60, GlobalText[565]);
            g_pRenderText->RenderText(90, 164 - 45, GlobalText[566]);
            g_pRenderText->RenderText(90, 164 - 30, GlobalText[567]);
        }
    }
}