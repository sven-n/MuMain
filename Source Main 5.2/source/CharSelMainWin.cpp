//*****************************************************************************
// File: CharSelMainWin.cpp
//*****************************************************************************

#include "MUMain/stdafx.h"
#include "CharSelMainWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "UIGuildInfo.h"
#include "ZzzOpenData.h"
#include "ZzzOpenglUtil.h"
#include "ServerListManager.h"

#include <algorithm>
#include <utility>

namespace
{
    constexpr int kCharacterSlotCount = 5;
    constexpr int kButtonSpacing = 1;
    constexpr int kInfoSpacing = 2;
    constexpr int kInfoOffsetY = 5;
    constexpr int kStatPanelBaseXOffset = 346;
    constexpr int kStatPanelOffsetY = 24;
    constexpr int kJobButtonsStartY = 131;
    constexpr int kRageFighterButtonsY = 246;
    constexpr int kSummonerRow = 3;
    constexpr int kActionButtonsRowOffsetY = 325;
    constexpr int kCancelButtonOffsetX = 400;
    constexpr int kInputSpriteOffsetY = 317;
    constexpr int kInputTextOffsetX = 78;
    constexpr int kInputTextOffsetY = 21;
    constexpr int kDescriptionSpriteOffsetY = 355;
    constexpr int kDecorOffsetX = 22;
    constexpr int kDecorOffsetY = 59;
    constexpr int kAccountBlockMsgX = 320;
    constexpr int kAccountBlockPrimaryY = 330;
    constexpr int kAccountBlockSecondaryY = 348;
    constexpr int kWindowAlpha = 143;
    constexpr int kInfoSpriteHeight = 21;

    template <typename Predicate>
    bool AnyCharacter(Predicate&& predicate)
    {
        return std::any_of(
            CharactersClient,
            CharactersClient + kCharacterSlotCount,
            std::forward<Predicate>(predicate));
    }

    bool HasAccountBlockedCharacter()
    {
        return AnyCharacter([](const CHARACTER& character)
        {
            return character.Object.Live != 0
                && (character.CtlCode & CTLCODE_10ACCOUNT_BLOCKITEM);
        });
    }

    bool HasEmptyCharacterSlot()
    {
        return AnyCharacter([](const CHARACTER& character)
        {
            return character.Object.Live == 0;
        });
    }

    bool HasLiveCharacter()
    {
        return AnyCharacter([](const CHARACTER& character)
        {
            return character.Object.Live != 0;
        });
    }

    CHARACTER* GetSelectedCharacter()
    {
        if (SelectedHero < 0 || SelectedHero >= kCharacterSlotCount)
            return nullptr;
        return &CharactersClient[SelectedHero];
    }

    void RenderAccountBlockMessage()
    {
        g_pRenderText->SetTextColor(0, 0, 0, 255);
        g_pRenderText->SetBgColor(255, 255, 0, 128);
        g_pRenderText->RenderText(kAccountBlockMsgX, kAccountBlockPrimaryY, GlobalText[436], 0, 0, RT3_WRITE_CENTER);
        g_pRenderText->RenderText(kAccountBlockMsgX, kAccountBlockSecondaryY, GlobalText[437], 0, 0, RT3_WRITE_CENTER);
    }
}

CCharSelMainWin::CCharSelMainWin()
{
}

CCharSelMainWin::~CCharSelMainWin()
{
}

void CCharSelMainWin::Create()
{
    CInput& input = CInput::Instance();

    m_asprBack[CSMW_SPR_DECO].Create(189, 103, BITMAP_LOG_IN + 2);
    m_asprBack[CSMW_SPR_INFO].Create(
        input.GetScreenWidth() - 266, kInfoSpriteHeight);
    m_asprBack[CSMW_SPR_INFO].SetColor(0, 0, 0);
    m_asprBack[CSMW_SPR_INFO].SetAlpha(kWindowAlpha);

    m_aBtn[CSMW_BTN_CREATE].Create(54, 30, BITMAP_LOG_IN + 3, 4, 2, 1, 3);
    m_aBtn[CSMW_BTN_MENU].Create(54, 30, BITMAP_LOG_IN + 4, 3, 2, 1);
    m_aBtn[CSMW_BTN_CONNECT].Create(54, 30, BITMAP_LOG_IN + 5, 4, 2, 1, 3);
    m_aBtn[CSMW_BTN_DELETE].Create(54, 30, BITMAP_LOG_IN + 6, 4, 2, 1, 3);

    CWin::Create(
        m_aBtn[0].GetWidth() * CSMW_BTN_MAX + m_asprBack[CSMW_SPR_INFO].GetWidth() + 6,
        m_aBtn[0].GetHeight(), -2);

    for (int i = 0; i < CSMW_BTN_MAX; ++i)
        CWin::RegisterButton(&m_aBtn[i]);

    m_bAccountBlockItem = HasAccountBlockedCharacter();
}

void CCharSelMainWin::PreRelease()
{
    for (int i = 0; i < CSMW_SPR_MAX; ++i)
        m_asprBack[i].Release();
}

void CCharSelMainWin::SetPosition(int nXCoord, int nYCoord)
{
    CWin::SetPosition(nXCoord, nYCoord);

    const int buttonWidth = m_aBtn[0].GetWidth();

    m_aBtn[CSMW_BTN_CREATE].SetPosition(nXCoord, nYCoord);
    m_aBtn[CSMW_BTN_MENU].SetPosition(nXCoord + buttonWidth + kButtonSpacing, nYCoord);

    const int infoX = m_aBtn[CSMW_BTN_MENU].GetXPos() + buttonWidth + kInfoSpacing;
    m_asprBack[CSMW_SPR_INFO].SetPosition(infoX, nYCoord + kInfoOffsetY);

    const int windowRightX = nXCoord + CWin::GetWidth();
    m_asprBack[CSMW_SPR_DECO].SetPosition(windowRightX - (m_asprBack[CSMW_SPR_DECO].GetWidth() - kDecorOffsetX), nYCoord - kDecorOffsetY);

    m_aBtn[CSMW_BTN_DELETE].SetPosition(windowRightX - buttonWidth, nYCoord);
    m_aBtn[CSMW_BTN_CONNECT].SetPosition(windowRightX - (buttonWidth * 2 + kButtonSpacing), nYCoord);
}

void CCharSelMainWin::Show(bool bShow)
{
    CWin::Show(bShow);

    for (auto& sprite : m_asprBack)
        sprite.Show(bShow);
    for (auto& button : m_aBtn)
        button.Show(bShow);
}

bool CCharSelMainWin::CursorInWin(int nArea)
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

void CCharSelMainWin::UpdateDisplay()
{
    m_aBtn[CSMW_BTN_CREATE].SetEnable(HasEmptyCharacterSlot());

    const bool hasSelection = (SelectedHero > -1);
    m_aBtn[CSMW_BTN_CONNECT].SetEnable(hasSelection);
    m_aBtn[CSMW_BTN_DELETE].SetEnable(hasSelection);

    if (!HasLiveCharacter())
    {
        CUIMng& rUIMng = CUIMng::Instance();
        rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
    }
}

void CCharSelMainWin::UpdateWhileActive(double dDeltaTick)
{
    CUIMng& uiManager = CUIMng::Instance();

    if (m_aBtn[CSMW_BTN_CONNECT].IsClick())
    {
        ::StartGame();
    }
    else if (m_aBtn[CSMW_BTN_MENU].IsClick())
    {
        uiManager.ShowWin(&uiManager.m_SysMenuWin);
        uiManager.SetSysMenuWinShow(true);
    }
    else if (m_aBtn[CSMW_BTN_CREATE].IsClick())
    {
        uiManager.ShowWin(&uiManager.m_CharMakeWin);
    }
    else if (m_aBtn[CSMW_BTN_DELETE].IsClick())
    {
        DeleteCharacter();
    }
}

void CCharSelMainWin::RenderControls()
{
    for (auto& sprite : m_asprBack)
        sprite.Render();

    ::EnableAlphaTest();
    ::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->SetBgColor(0);

    if (m_bAccountBlockItem)
        RenderAccountBlockMessage();

    CWin::RenderButtons();
}

void CCharSelMainWin::DeleteCharacter()
{
    CHARACTER* selected = GetSelectedCharacter();
    if (selected == nullptr)
        return;

    CUIMng& uiManager = CUIMng::Instance();

    if (selected->GuildStatus != G_NONE)
    {
        uiManager.PopUpMsgWin(MESSAGE_DELETE_CHARACTER_GUILDWARNING);
    }
    else if (selected->CtlCode & (CTLCODE_02BLOCKITEM | CTLCODE_10ACCOUNT_BLOCKITEM))
    {
        uiManager.PopUpMsgWin(MESSAGE_DELETE_CHARACTER_ID_BLOCK);
    }
    else
    {
        uiManager.PopUpMsgWin(MESSAGE_DELETE_CHARACTER_CONFIRM);
    }
}