//*****************************************************************************
// File: CharSelMainWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CharSelMainWin.h"
#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Guild/UIGuildInfo.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Network/Server/ServerListManager.h"
#include "I18N/All.h"
#include "UI/Scaling/UITransform.h"

#include <algorithm>
#include <utility>

#include "Scenes/SceneCommon.h"

namespace
{
    constexpr int kCharacterSlotCount = 5;
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
    constexpr int kAccountBlockMsgX = 320;
    constexpr int kAccountBlockPrimaryY = 330;
    constexpr int kAccountBlockSecondaryY = 348;
    constexpr int kWindowAlpha = 143;

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
        g_pRenderText->RenderText(kAccountBlockMsgX, kAccountBlockPrimaryY, I18N::Game::ThisAccountIsItemBlocked, 0, 0, RT3_WRITE_CENTER);
        g_pRenderText->RenderText(kAccountBlockMsgX, kAccountBlockSecondaryY, I18N::Game::PleaseCheckOnHttpMuonlineWebzenComSite, 0, 0, RT3_WRITE_CENTER);
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
    const auto layout = UI::CharacterSelection::CalculateLayout(
        static_cast<int>(input.GetScreenWidth()), static_cast<int>(input.GetScreenHeight()));

    m_asprBack[CSMW_SPR_DECO].Create(
        UI::CharacterSelection::NativeDecorationWidth,
        UI::CharacterSelection::NativeDecorationHeight,
        BITMAP_LOG_IN + 2);
    m_asprBack[CSMW_SPR_INFO].Create(
        layout.information.width, UI::CharacterSelection::NativeInformationHeight);
    m_asprBack[CSMW_SPR_INFO].SetColor(0, 0, 0);
    m_asprBack[CSMW_SPR_INFO].SetAlpha(kWindowAlpha);

    m_aBtn[CSMW_BTN_CREATE].Create(
        UI::CharacterSelection::NativeButtonWidth,
        UI::CharacterSelection::NativeButtonHeight,
        BITMAP_LOG_IN + 3, 4, 2, 1, 3);
    m_aBtn[CSMW_BTN_MENU].Create(
        UI::CharacterSelection::NativeButtonWidth,
        UI::CharacterSelection::NativeButtonHeight,
        BITMAP_LOG_IN + 4, 3, 2, 1);
    m_aBtn[CSMW_BTN_CONNECT].Create(
        UI::CharacterSelection::NativeButtonWidth,
        UI::CharacterSelection::NativeButtonHeight,
        BITMAP_LOG_IN + 5, 4, 2, 1, 3);
    m_aBtn[CSMW_BTN_DELETE].Create(
        UI::CharacterSelection::NativeButtonWidth,
        UI::CharacterSelection::NativeButtonHeight,
        BITMAP_LOG_IN + 6, 4, 2, 1, 3);

    CWin::Create(layout.window.width, layout.window.height, -2);

    for (int i = 0; i < CSMW_BTN_MAX; ++i)
        CWin::RegisterButton(&m_aBtn[i]);

    ApplyLayout(layout);
    m_bAccountBlockItem = HasAccountBlockedCharacter();
}

void CCharSelMainWin::ApplyLayout(const UI::CharacterSelection::Layout& layout)
{
    CWin::SetSize(layout.window.width, layout.window.height);
    CWin::SetPosition(layout.window.x, layout.window.y);

    m_asprBack[CSMW_SPR_DECO].SetSize(layout.decoration.width, layout.decoration.height);
    m_asprBack[CSMW_SPR_DECO].SetPosition(layout.decoration.x, layout.decoration.y);
    m_asprBack[CSMW_SPR_INFO].SetSize(layout.information.width, layout.information.height);
    m_asprBack[CSMW_SPR_INFO].SetPosition(layout.information.x, layout.information.y);

    for (int i = 0; i < CSMW_BTN_MAX; ++i)
    {
        const auto& button = layout.buttons[static_cast<std::size_t>(i)];
        m_aBtn[i].SetSize(button.width, button.height);
        m_aBtn[i].SetPosition(button.x, button.y);
    }
}

void CCharSelMainWin::PreRelease()
{
    for (int i = 0; i < CSMW_SPR_MAX; ++i)
        m_asprBack[i].Release();
}

void CCharSelMainWin::SetPosition(int nXCoord, int nYCoord)
{
    const int deltaX = nXCoord - CWin::GetXPos();
    const int deltaY = nYCoord - CWin::GetYPos();
    CWin::SetPosition(nXCoord, nYCoord);

    for (auto& sprite : m_asprBack)
        sprite.SetPosition(sprite.GetXPos() + deltaX, sprite.GetYPos() + deltaY);
    for (auto& button : m_aBtn)
        button.SetPosition(button.GetXPos() + deltaX, button.GetYPos() + deltaY);
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
    const UI::Scaling::Transform physicalPixels{1.0f, 1.0f, 0.0f, 0.0f, 1.0f};
    {
        UI::Scaling::ScopedActiveTransform scoped(physicalPixels);
        for (auto& sprite : m_asprBack)
            sprite.Render();
    }

    ::EnableAlphaTest();
    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->SetBgColor(0);

    if (m_bAccountBlockItem)
        RenderAccountBlockMessage();

    UI::Scaling::ScopedActiveTransform scoped(physicalPixels);
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
