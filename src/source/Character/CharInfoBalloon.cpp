//*****************************************************************************
// File: CharInfoBalloon.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CharInfoBalloon.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Engine/Object/ZzzInterface.h"
#include "CharacterManager.h"
#include "I18N/All.h"

#include <algorithm>
#include <array>
#include <cwchar>

#include "Camera/CameraProjection.h"
#include "UI/Scaling/UITransform.h"

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
        const wchar_t* const* text;
    };

    constexpr GuildStatusText kGuildStatusTexts[] = {
        {0, &I18N::Game::Members},  {32, &I18N::Game::BattleM},   {64, &I18N::Game::AssistM},
        {128, &I18N::Game::Master}, {255, &I18N::Game::Commoner},
    };
    // GetGuildStatusText searches the table with std::lower_bound.
    static_assert(std::adjacent_find(std::begin(kGuildStatusTexts), std::end(kGuildStatusTexts),
                                     [](const GuildStatusText& left, const GuildStatusText& right)
                                     { return left.status >= right.status; }) == std::end(kGuildStatusTexts),
                  "kGuildStatusTexts is sorted by status, each status once");

    // The guild rank shown in the balloon for a character's GuildStatus, in the current language;
    // nullptr for a status that is no rank.
    const wchar_t* GetGuildStatusText(std::uint8_t guildStatus)
    {
        const auto it =
            std::lower_bound(std::begin(kGuildStatusTexts), std::end(kGuildStatusTexts), guildStatus,
                             [](const GuildStatusText& entry, std::uint8_t status) { return entry.status < status; });

        return (it != std::end(kGuildStatusTexts) && it->status == guildStatus) ? *it->text : nullptr;
    }

    BalloonNameStatus ResolveNameStatus(std::uint8_t controlCode)
    {
        if (controlCode & CTLCODE_01BLOCKCHAR)
            return BalloonNameStatus::BlockedCharacter;
        if (controlCode & (CTLCODE_02BLOCKITEM | CTLCODE_10ACCOUNT_BLOCKITEM))
            return BalloonNameStatus::BlockedItems;
        if (controlCode & CTLCODE_04FORTV)
            return BalloonNameStatus::Normal;
        if (controlCode & (CTLCODE_08OPERATOR | CTLCODE_20OPERATOR))
            return BalloonNameStatus::Operator;

        return BalloonNameStatus::Normal;
    }
}

CCharInfoBalloon::CCharInfoBalloon() : m_pCharInfo(nullptr)
{
    I18N::RegisterLocaleObserver(&CCharInfoBalloon::OnLocaleChanged, this);
}

CCharInfoBalloon::~CCharInfoBalloon()
{
    I18N::UnregisterLocaleObserver(&CCharInfoBalloon::OnLocaleChanged, this);
}

void CCharInfoBalloon::OnLocaleChanged(void* ctx) noexcept
{
    auto* self = static_cast<CCharInfoBalloon*>(ctx);
    if (self->m_pCharInfo != nullptr)
    {
        self->SetInfo();
    }
}

void CCharInfoBalloon::Create(CHARACTER* pCharInfo)
{
    CSprite::Create(118, 54, BITMAP_LOG_IN + 7, 0, nullptr, 59, 54);

    m_pCharInfo = pCharInfo;
    m_eNameStatus = BalloonNameStatus::Normal;
    std::fill(std::begin(m_szName), std::end(m_szName), L'\0');
    std::fill(std::begin(m_szGuild), std::end(m_szGuild), L'\0');
    std::fill(std::begin(m_szClass), std::end(m_szClass), L'\0');
}

void CCharInfoBalloon::Render()
{
    // No longer draws anything (RmlUi owns 100% of this balloon's visuals -- see this class's
    // header comment) -- still recomputes the live screen position every call, since the
    // character's world position (and the camera) can change frame to frame. CSprite::SetPosition
    // keeps this sprite's own GetXPos()/GetYPos() bookkeeping in sync so
    // CCharInfoBalloonMng::SyncRmlModel() can read it back right after calling this.
    if (m_pCharInfo == nullptr || !CSprite::m_bShow)
        return;

    vec3_t afPos;
    VectorCopy(m_pCharInfo->Object.Position, afPos);
    afPos[2] += 350.0f;

    int nPosX, nPosY;
    CameraProjection::WorldToScreen(g_Camera, afPos, &nPosX, &nPosY);

    // WorldToScreen() reports the original's screen stretched over the window.
    const UI::Scaling::Transform screen =
        UI::Scaling::ScreenOverlayTransform(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    CSprite::SetPosition(static_cast<int>(UI::Scaling::SizeX(screen, static_cast<float>(nPosX))),
                         static_cast<int>(UI::Scaling::SizeY(screen, static_cast<float>(nPosY))));
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

    m_eNameStatus = ResolveNameStatus(m_pCharInfo->CtlCode);

    CopyWideString(m_szName, m_pCharInfo->ID);

    // A guild status without a rank text (OpenMU sends only the five ranks) shows no rank.
    const wchar_t* guildStatusText = GetGuildStatusText(m_pCharInfo->GuildStatus);
    if (guildStatusText != nullptr)
        mu_swprintf_s(m_szGuild, L"(%ls)", guildStatusText);
    else
        m_szGuild[0] = L'\0';
    mu_swprintf_s(m_szClass, L"%ls %d",
        gCharacterManager.GetCharacterClassText(m_pCharInfo->Class),
        m_pCharInfo->Level);
}