// NewUIDoppelGangerFrame.cpp: implementation of the CNewUIDoppelGangerFrame class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIDoppelGangerFrame.h"
#include "NewUISystem.h"

using namespace SEASON3B;

CNewUIDoppelGangerFrame::CNewUIDoppelGangerFrame()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iEnteredMonsters = 0;
    m_iMaxMonsters = 0;
    m_fMonsterGauge = 0.0f;
    m_fMonsterGaugeRcvd = 0.0f;
    m_iTime = 1800;
    m_bStopTimer = FALSE;
    m_bIceWalkerEnabled = FALSE;
    m_fIceWalkerPositionRcvd = 0.0f;
    m_fIceWalkerPosition = 0.0f;
}

CNewUIDoppelGangerFrame::~CNewUIDoppelGangerFrame()
{
    Release();
}

bool CNewUIDoppelGangerFrame::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_DOPPELGANGER_FRAME, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void CNewUIDoppelGangerFrame::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIDoppelGangerFrame::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUIDoppelGangerFrame::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;
    return true;
}

bool CNewUIDoppelGangerFrame::UpdateKeyEvent()
{
    return true;
}

bool CNewUIDoppelGangerFrame::Update()
{
    if (!IsVisible())
        return true;

    return true;
}

bool CNewUIDoppelGangerFrame::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderImage(IMAGE_DOPPELGANGER_FRAME_WINDOW, m_Pos.x, m_Pos.y, float(DOPPELGANGER_FRAME_WINDOW_WIDTH), float(DOPPELGANGER_FRAME_WINDOW_HEIGHT));

    wchar_t szText[256] = { NULL, };
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);

    if (m_iEnteredMonsters == 0)
    {
        g_pRenderText->SetTextColor(255, 150, 0, 255);
    }
    else if (m_iEnteredMonsters == 1)
    {
        g_pRenderText->SetTextColor(255, 70, 0, 255);
    }
    else if (m_iEnteredMonsters >= 2)
    {
        g_pRenderText->SetTextColor(255, 0, 0, 255);
    }

    swprintf(szText, GlobalText[2772], m_iEnteredMonsters, m_iMaxMonsters);
    g_pRenderText->RenderText(m_Pos.x + 117, m_Pos.y + 13, szText, 110, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(255, 150, 0, 255);
    g_pRenderText->RenderText(m_Pos.x + 117, m_Pos.y + 38, GlobalText[865], 110, 0, RT3_SORT_CENTER);

    int iMinute = m_iTime / 60;
    int iSecond = 99 - (int)WorldTime % 100;
    if (m_bStopTimer == TRUE)
    {
        iSecond = 0;
    }

    swprintf(szText, L"%.2d:%.2d:%.2d", iMinute, m_iTime % 60, iSecond);
    g_pRenderText->SetFont(g_hFontBig);
    g_pRenderText->RenderText(m_Pos.x + 117, m_Pos.y + 50, szText, 110, 0, RT3_SORT_CENTER);

    float fMonsterGaugeSpeed = 0.01f;
    if (m_fMonsterGauge + fMonsterGaugeSpeed <= m_fMonsterGaugeRcvd)
    {
        m_fMonsterGauge += fMonsterGaugeSpeed;
    }
    else if (m_fMonsterGauge - fMonsterGaugeSpeed >= m_fMonsterGaugeRcvd)
    {
        m_fMonsterGauge -= fMonsterGaugeSpeed;
    }
    else
    {
        m_fMonsterGauge = m_fMonsterGaugeRcvd;
    }

    if (m_iEnteredMonsters == 0)
    {
        RenderImage(IMAGE_DOPPELGANGER_GUAGE_YELLOW, m_Pos.x + 59 + 167.f * (1.0f - m_fMonsterGauge), m_Pos.y + 78,
            167.f * m_fMonsterGauge, 8.f - 1, 165.f / 256.f * (1.0f - m_fMonsterGauge), 0, 165.f / 256.f * m_fMonsterGauge, 6.f / 8.f);
    }
    else if (m_iEnteredMonsters == 1)
    {
        RenderImage(IMAGE_DOPPELGANGER_GUAGE_YELLOW, m_Pos.x + 59, m_Pos.y + 78,
            167.f, 8.f - 1, 0, 0, 165.f / 256.f, 6.f / 8.f);
        RenderImage(IMAGE_DOPPELGANGER_GUAGE_ORANGE, m_Pos.x + 59 + 167.f * (1.0f - m_fMonsterGauge), m_Pos.y + 78,
            167.f * m_fMonsterGauge, 8.f - 1, 165.f / 256.f * (1.0f - m_fMonsterGauge), 0, 165.f / 256.f * m_fMonsterGauge, 6.f / 8.f);
    }
    else if (m_iEnteredMonsters >= 2)
    {
        RenderImage(IMAGE_DOPPELGANGER_GUAGE_ORANGE, m_Pos.x + 59, m_Pos.y + 78,
            167.f, 8.f - 1, 0, 0, 165.f / 256.f, 6.f / 8.f);
        RenderImage(IMAGE_DOPPELGANGER_GUAGE_RED, m_Pos.x + 59 + 167.f * (1.0f - m_fMonsterGauge), m_Pos.y + 78,
            167.f * m_fMonsterGauge, 8.f - 1, 165.f / 256.f * (1.0f - m_fMonsterGauge), 0, 165.f / 256.f * m_fMonsterGauge, 6.f / 8.f);
    }

    if (m_bIceWalkerEnabled == TRUE)
    {
        if (m_fIceWalkerPosition + fMonsterGaugeSpeed <= m_fIceWalkerPositionRcvd)
        {
            m_fIceWalkerPosition += fMonsterGaugeSpeed;
        }
        else if (m_fIceWalkerPosition - fMonsterGaugeSpeed >= m_fIceWalkerPositionRcvd)
        {
            m_fIceWalkerPosition -= fMonsterGaugeSpeed;
        }
        else
        {
            m_fIceWalkerPosition = m_fIceWalkerPositionRcvd;
        }

        RenderImage(IMAGE_DOPPELGANGER_GUAGE_ICEWALKER, m_Pos.x + 59 - 6.5f + 167 * m_fIceWalkerPosition, m_Pos.y + 78 - 1, 13.0f, 7.0f);
    }

    for (std::map<WORD, PARTY_POSITION>::iterator iter = m_PartyPositionMap.begin(); iter != m_PartyPositionMap.end(); ++iter)
    {
        if (iter->second.m_fPositionRcvd == -1) continue;

        float fPartyPositionSpeed = 0.01f;
        if (iter->second.m_fPosition + fPartyPositionSpeed <= iter->second.m_fPositionRcvd)
        {
            iter->second.m_fPosition += fPartyPositionSpeed;
        }
        else if (iter->second.m_fPosition - fPartyPositionSpeed >= iter->second.m_fPositionRcvd)
        {
            iter->second.m_fPosition -= fPartyPositionSpeed;
        }
        else
        {
            iter->second.m_fPosition = iter->second.m_fPositionRcvd;
        }

        if (iter->first == Hero->Key)
        {
            RenderImage(IMAGE_DOPPELGANGER_GUAGE_PLAYER, m_Pos.x + 59 - 4.5f + 167 * iter->second.m_fPosition, m_Pos.y + 78 + 1, 9.0f, 8.0f);
        }
        else
        {
            RenderImage(IMAGE_DOPPELGANGER_GUAGE_PARTY_MEMBER, m_Pos.x + 59 - 4.5f + 167 * iter->second.m_fPosition, m_Pos.y + 78 + 1, 9.0f, 8.0f);
        }
    }

    DisableAlphaBlend();

    return true;
}

bool CNewUIDoppelGangerFrame::BtnProcess()
{
    return false;
}

float CNewUIDoppelGangerFrame::GetLayerDepth()
{
    return 1.2f;
}

void CNewUIDoppelGangerFrame::OpenningProcess()
{
    m_iEnteredMonsters = 0;
    m_iMaxMonsters = 0;
    m_fMonsterGauge = 0.0f;
    m_fMonsterGaugeRcvd = 0.0f;
    m_iTime = 600;
    m_bStopTimer = FALSE;
    ResetPartyMemberInfo();
    m_fIceWalkerPositionRcvd = 0.0f;
    m_fIceWalkerPosition = 0.0f;
    m_bIceWalkerEnabled = FALSE;

    EnabledDoppelGangerEvent(TRUE);
}

void CNewUIDoppelGangerFrame::ClosingProcess()
{
    EnabledDoppelGangerEvent(FALSE);
}

void CNewUIDoppelGangerFrame::LoadImages()
{
    LoadBitmap(L"Interface\\Double_back.tga", IMAGE_DOPPELGANGER_FRAME_WINDOW, GL_LINEAR);
    LoadBitmap(L"Interface\\Double_bar(R).jpg", IMAGE_DOPPELGANGER_GUAGE_RED, GL_LINEAR);
    LoadBitmap(L"Interface\\Double_bar(O).jpg", IMAGE_DOPPELGANGER_GUAGE_ORANGE, GL_LINEAR);
    LoadBitmap(L"Interface\\Double_bar(Y).jpg", IMAGE_DOPPELGANGER_GUAGE_YELLOW, GL_LINEAR);
    LoadBitmap(L"Interface\\Double_Baricon04.tga", IMAGE_DOPPELGANGER_GUAGE_PLAYER, GL_LINEAR);
    LoadBitmap(L"Interface\\Double_Baricon01.tga", IMAGE_DOPPELGANGER_GUAGE_PARTY_MEMBER, GL_LINEAR);
    LoadBitmap(L"Interface\\Double_Micon01.tga", IMAGE_DOPPELGANGER_GUAGE_ICEWALKER, GL_LINEAR);
}

void CNewUIDoppelGangerFrame::UnloadImages()
{
    DeleteBitmap(IMAGE_DOPPELGANGER_FRAME_WINDOW);
}

void CNewUIDoppelGangerFrame::SetMonsterGauge(float fValue)
{
    m_fMonsterGaugeRcvd = fValue;
}

void CNewUIDoppelGangerFrame::SetRemainTime(int iSeconds)
{
    m_iTime = iSeconds;
}

void CNewUIDoppelGangerFrame::StopTimer(BOOL bFlag)
{
    m_bStopTimer = bFlag;
}

void CNewUIDoppelGangerFrame::ResetPartyMemberInfo()
{
    m_PartyPositionMap.clear();
}

void CNewUIDoppelGangerFrame::SetPartyMemberRcvd()
{
    for (std::map<WORD, PARTY_POSITION>::iterator iter = m_PartyPositionMap.begin(); iter != m_PartyPositionMap.end(); ++iter)
    {
        iter->second.m_fPositionRcvd = -1.0f;
    }
}

void CNewUIDoppelGangerFrame::SetPartyMemberInfo(WORD wIndex, float fPosition)
{
    auto iter = m_PartyPositionMap.find(wIndex);
    if (iter == m_PartyPositionMap.end())
    {
        PARTY_POSITION party_pos;
        party_pos.m_fPosition = 0;
        party_pos.m_fPositionRcvd = fPosition;
        m_PartyPositionMap.insert(std::pair<WORD, PARTY_POSITION>(wIndex, party_pos));
    }
    else
    {
        iter->second.m_fPositionRcvd = fPosition;
    }
}

void CNewUIDoppelGangerFrame::SetIceWalkerMap(BOOL bEnable, float fPosition)
{
    if (bEnable == TRUE)
    {
        if (m_bIceWalkerEnabled == TRUE)
        {
            m_fIceWalkerPositionRcvd = fPosition;
        }
        else
        {
            m_fIceWalkerPositionRcvd = fPosition;
            m_fIceWalkerPosition = fPosition;
        }
        m_bIceWalkerEnabled = TRUE;
    }
    else
    {
        m_bIceWalkerEnabled = FALSE;
        m_fIceWalkerPositionRcvd = 0.0f;
    }
}