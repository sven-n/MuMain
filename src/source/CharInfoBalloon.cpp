//*****************************************************************************
// File: CharInfoBalloon.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CharInfoBalloon.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInterface.h"
#include "UIControls.h"
#include "CharacterManager.h"

#include <algorithm>
#include <array>
#include <cwchar>

namespace
{
    template <std::size_t N>
    void CopyWideString(wchar_t (&destination)[N], const wchar_t* source)
    {
        if (source == nullptr)
        {
            destination[0] = L'\0';
            return;
        }

        std::wcsncpy(destination, source, N - 1);
        destination[N - 1] = L'\0';
    }

    struct GuildStatusText
    {
        std::uint8_t status;
        int textIndex;
    };

    constexpr std::array<GuildStatusText, 5> kGuildStatusTexts{ {
        {   0, 1330 },
        {  32, 1302 },
        {  64, 1301 },
        { 128, 1300 },
        { 255, 488 },
    } };

    DWORD ResolveNameColor(std::uint8_t controlCode)
    {
        if (controlCode & CTLCODE_01BLOCKCHAR)
            return ARGB(255, 0, 255, 255);
        if (controlCode & (CTLCODE_02BLOCKITEM | CTLCODE_10ACCOUNT_BLOCKITEM))
            return CLRDW_BR_ORANGE;
        if (controlCode & CTLCODE_04FORTV)
            return CLRDW_WHITE;
        if (controlCode & (CTLCODE_08OPERATOR | CTLCODE_20OPERATOR))
            return ARGB(255, 255, 0, 0);

        return CLRDW_WHITE;
    }

    int ResolveGuildTextIndex(std::uint8_t guildStatus)
    {
        const auto it = std::lower_bound(
            kGuildStatusTexts.begin(),
            kGuildStatusTexts.end(),
            guildStatus,
            [](const GuildStatusText& entry, std::uint8_t status) { return entry.status < status; });

        return (it != kGuildStatusTexts.end() && it->status == guildStatus) ? it->textIndex : 0;
    }
}

CCharInfoBalloon::CCharInfoBalloon() : m_pCharInfo(nullptr)
{
}

CCharInfoBalloon::~CCharInfoBalloon()
{
}

void CCharInfoBalloon::Create(CHARACTER* pCharInfo)
{
    CSprite::Create(118, 54, BITMAP_LOG_IN + 7, 0, nullptr, 59, 54);

    m_pCharInfo = pCharInfo;
    m_dwNameColor = 0;
    std::fill(std::begin(m_szName), std::end(m_szName), L'\0');
    std::fill(std::begin(m_szGuild), std::end(m_szGuild), L'\0');
    std::fill(std::begin(m_szClass), std::end(m_szClass), L'\0');
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
    if (m_pCharInfo == nullptr)
        return;

    if (!m_pCharInfo->Object.Live)
    {
        CSprite::m_bShow = false;
        return;
    }

    CSprite::m_bShow = true;

    m_dwNameColor = ResolveNameColor(m_pCharInfo->CtlCode);

    CopyWideString(m_szName, m_pCharInfo->ID);

    const int guildTextIndex = ResolveGuildTextIndex(m_pCharInfo->GuildStatus);
    mu_swprintf_s(m_szGuild, L"(%ls)", GlobalText[guildTextIndex]);
    mu_swprintf_s(m_szClass, L"%ls %d",
        gCharacterManager.GetCharacterClassText(m_pCharInfo->Class),
        m_pCharInfo->Level);
}