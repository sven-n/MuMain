//*****************************************************************************
// File: CharInfoBalloon.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CharInfoBalloon.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInterface.h"
#include "UIControls.h"
#include "CharacterManager.h"




CCharInfoBalloon::CCharInfoBalloon() : m_pCharInfo(NULL)
{
}

CCharInfoBalloon::~CCharInfoBalloon()
{
}

void CCharInfoBalloon::Create(CHARACTER* pCharInfo)
{
    CSprite::Create(118, 54, BITMAP_LOG_IN + 7, 0, NULL, 59, 54);

    m_pCharInfo = pCharInfo;
    m_dwNameColor = 0;
    memset(m_szName, 0, sizeof(char) * 64);
    memset(m_szGuild, 0, sizeof(char) * 64);
    memset(m_szClass, 0, sizeof(char) * 64);
}

void CCharInfoBalloon::Render()
{
    if (m_pCharInfo == nullptr || !CSprite::m_bShow)
        return;

    CSprite::Render();

    vec3_t afPos;
    VectorCopy(m_pCharInfo->Object.Position, afPos);
    afPos[2] += 350.0f;

    int nPosX, nPosY;
    ::Projection(afPos, &nPosX, &nPosY);

    CSprite::SetPosition(
        int(nPosX * g_fScreenRate_x),
        int(nPosY * g_fScreenRate_y)
    );

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetBgColor(0);

    const int spriteX = CSprite::GetXPos();
    const int spriteY = CSprite::GetYPos();
    const int spriteW = CSprite::GetWidth();

    const int nTextPosX = int(spriteX / g_fScreenRate_x);

    g_pRenderText->SetTextColor(m_dwNameColor);
    g_pRenderText->RenderText(
        nTextPosX,
        int((spriteY + 6) / g_fScreenRate_y),
        m_szName,
        spriteW / g_fScreenRate_x,
        0,
        RT3_SORT_CENTER
    );

    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->RenderText(
        nTextPosX,
        int((spriteY + 22) / g_fScreenRate_y),
        m_szGuild,
        spriteW / g_fScreenRate_x,
        0,
        RT3_SORT_CENTER
    );

    g_pRenderText->SetTextColor(CLRDW_BR_ORANGE);
    g_pRenderText->RenderText(
        nTextPosX,
        int((spriteY + 38) / g_fScreenRate_y),
        m_szClass,
        spriteW / g_fScreenRate_x,
        0,
        RT3_SORT_CENTER
    );
}

void CCharInfoBalloon::SetInfo()
{
    if (NULL == m_pCharInfo)
        return;

    if (!m_pCharInfo->Object.Live)
    {
        CSprite::m_bShow = false;
        return;
    }

    CSprite::m_bShow = true;

    if (m_pCharInfo->CtlCode & CTLCODE_01BLOCKCHAR)
        m_dwNameColor = ARGB(255, 0, 255, 255);
    else if (m_pCharInfo->CtlCode
        & (CTLCODE_02BLOCKITEM | CTLCODE_10ACCOUNT_BLOCKITEM))
        m_dwNameColor = CLRDW_BR_ORANGE;
    else if (m_pCharInfo->CtlCode & CTLCODE_04FORTV)
        m_dwNameColor = CLRDW_WHITE;
    else if (m_pCharInfo->CtlCode & CTLCODE_08OPERATOR)
        m_dwNameColor = ARGB(255, 255, 0, 0);
    else if (m_pCharInfo->CtlCode & CTLCODE_20OPERATOR)
        m_dwNameColor = ARGB(255, 255, 0, 0);
    else
        m_dwNameColor = CLRDW_WHITE;

    wcscpy(m_szName, m_pCharInfo->ID);

    int nText = 0;
    switch (m_pCharInfo->GuildStatus)
    {
        case 255:	nText = 488;	break;
        case 0:		nText = 1330;	break;
        case 32:	nText = 1302;	break;
        case 64:	nText = 1301;	break;
        case 128:	nText = 1300;	break;
    }
    swprintf(m_szGuild, L"(%s)", GlobalText[nText]);
    swprintf(m_szClass, L"%s %d", gCharacterManager.GetCharacterClassText(m_pCharInfo->Class), m_pCharInfo->Level);
}