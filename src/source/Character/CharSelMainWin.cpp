//*****************************************************************************
// File: CharSelMainWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CharSelMainWin.h"
#include "Core/Input/Input.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Windows/SysMenuWin.h"
#include "Character/CharMakeWin.h"
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
#include "Core/Globals/_enum.h"

#include <algorithm>
#include <utility>

#include "Scenes/SceneCommon.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

extern unsigned int WindowWidth, WindowHeight;

namespace
{
    constexpr int kCharacterSlotCount = 5;
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

    // The three windows a modal open over CHARACTER_SCENE can be -- see this window's own header
    // comment and docs/newui-legacy-merger.md for why this window's own Update() must check this.
    bool IsCharacterSceneModalOpen()
    {
        return g_CharMakeWin.IsVisible() || g_MsgWin.IsVisible() || g_SysMenuWin.IsVisible();
    }
}

CCharSelMainWin g_CharSelMainWin;

CCharSelMainWin::CCharSelMainWin()
{
}

CCharSelMainWin::~CCharSelMainWin()
{
    Release();
}

void CCharSelMainWin::Create()
{
    Release();

    // WindowWidth/WindowHeight (ZzzOpenglUtil.cpp), not
    // CInput::Instance().GetScreenWidth()/GetScreenHeight() -- see LoginWin.cpp's
    // LoginUIScaleRatio() for why: a real, screenshot-confirmed bug traced back to CInput's own
    // copy of the screen size not reliably matching WindowWidth/WindowHeight (the exact values
    // RmlUiRuntime::OnResize() uses), fixed there and proactively fixed here too -- both the
    // layout dimensions and the ratio need the authoritative real window size, not just the ratio.
    const int screenWidth = static_cast<int>(WindowWidth);
    const int screenHeight = static_cast<int>(WindowHeight);
    const auto layout = UI::CharacterSelection::CalculateFixedAnchorLayout(
        screenWidth, screenHeight, UI::Scaling::CompanionRatio(screenWidth, screenHeight));

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

    m_ptPos.x = m_ptPos.y = 0;
    m_Size.cx = m_Size.cy = 0;

    // RmlUi migration -- see this class's header comment. Guarded the same way CLoginWin::Create()
    // is (CSceneUICoordinator::RepositionSceneUI() re-runs Create() on resolution change), so the document/model
    // are created once, ever, and only repositioned/resized/re-synced afterward. Must run BEFORE
    // ApplyLayout() below -- that call pushes the computed rects into the RmlUi elements too, and
    // does nothing on a null m_pRmlDoc, so calling it first (as an earlier version of this method
    // briefly did, after rebasing onto upstream's own Create() ordering) left every RmlUi element
    // at its unstyled default position on the window's very first Create() call.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "char_sel_main",
            [this](Rml::DataModelConstructor& c, CharSelMainRmlModel& model)
            {
                c.Bind("create_disabled", &model.createDisabled);
                c.Bind("connect_disabled", &model.connectDisabled);
                c.Bind("delete_disabled", &model.deleteDisabled);
                c.Bind("account_block_hidden", &model.accountBlockHidden);
                c.Bind("account_block_line1", &model.accountBlockLine1);
                c.Bind("account_block_line2", &model.accountBlockLine2);

                c.BindEventCallback("charsel_create_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickCreate(); });
                c.BindEventCallback("charsel_menu_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickMenu(); });
                c.BindEventCallback("charsel_connect_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickConnect(); });
                c.BindEventCallback("charsel_delete_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickDelete(); });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/char_sel_main.rml");
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(SEASON3B::INTERFACE_CHAR_SEL_MAIN, this);

    ApplyLayout(layout);
    m_bAccountBlockItem = HasAccountBlockedCharacter();

    Show(false);
}

void CCharSelMainWin::ApplyLayout(const UI::CharacterSelection::Layout& layout)
{
    m_Size.cx = layout.window.width;
    m_Size.cy = layout.window.height;
    m_ptPos.x = layout.window.x;
    m_ptPos.y = layout.window.y;

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

    // Deliberately does NOT push anything to the RmlUi elements (an earlier version of this
    // method did). 2026-08-31 retrofit (docs/rmlui-ui-system/layout-and-scaling.md): #panel and
    // every child position themselves via base.rcss's anchor-*/stretch-x utility classes with a
    // fixed `dp` size (char_sel_main.rcss) instead. `layout` above (now
    // CalculateFixedAnchorLayout(), not the old resolution-proportional CalculateLayout() --
    // see that function's own comment for why the switch was necessary, not just stylistic) still
    // feeds the legacy CSprite/CButton objects, which genuinely still need real screen-pixel rects
    // for their own hit-testing/UpdateMouseEvent() bookkeeping -- but those rects are now derived
    // from the SAME fixed-dp-anchor math as the RmlUi visuals, so the two stay pixel-for-pixel
    // aligned at every resolution and UI-scale setting instead of just at the historical
    // 800x600/100% case.
}

void CCharSelMainWin::Release()
{
    for (auto& sprite : m_asprBack)
        sprite.Release();
    for (auto& button : m_aBtn)
        button.Release();

    // See CLoginMainWin::PreRelease()'s identical comment -- each migrated window's Release() is
    // called explicitly at every scene transition, not swept automatically by any shared list, and
    // this class has no base-class knowledge of m_pRmlDoc, so without this it can keep rendering
    // into whatever scene comes next if this window happened to be open at the moment of
    // transition.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CCharSelMainWin::SetPosition(int nXCoord, int nYCoord)
{
    const int deltaX = nXCoord - m_ptPos.x;
    const int deltaY = nYCoord - m_ptPos.y;
    m_ptPos.x = nXCoord;
    m_ptPos.y = nYCoord;

    for (auto& sprite : m_asprBack)
        sprite.SetPosition(sprite.GetXPos() + deltaX, sprite.GetYPos() + deltaY);
    for (auto& button : m_aBtn)
        button.SetPosition(button.GetXPos() + deltaX, button.GetYPos() + deltaY);

    // No RmlUi push here (an earlier version had one) -- see ApplyLayout()'s comment. #panel is
    // a fixed full-screen container now (char_sel_main.rcss), not something this window's own
    // screen position moves; the RmlUi visuals are positioned independently via anchor classes.
    // No call site actually invokes this method today (CSceneUICoordinator::RepositionSceneUI() re-runs
    // Create() wholesale instead) -- the legacy CSprite/CButton delta-shift above is kept
    // correct anyway since it was part of CWin's public contract before this migration.
}

void CCharSelMainWin::Show(bool bShow)
{
    SEASON3B::CNewUIObj::Show(bShow);

    for (auto& sprite : m_asprBack)
        sprite.Show(bShow);
    for (auto& button : m_aBtn)
        button.Show(bShow);

    if (m_pRmlDoc)
    {
        if (bShow) { SyncRmlModel(); m_pRmlDoc->Show(); }
        else       m_pRmlDoc->Hide();
    }
}

void CCharSelMainWin::UpdateDisplay()
{
    m_bCreateEnabled = HasEmptyCharacterSlot();
    m_aBtn[CSMW_BTN_CREATE].SetEnable(m_bCreateEnabled);

    const bool hasSelection = (SelectedHero > -1);
    m_bConnectEnabled = hasSelection;
    m_bDeleteEnabled = hasSelection;
    m_aBtn[CSMW_BTN_CONNECT].SetEnable(m_bConnectEnabled);
    m_aBtn[CSMW_BTN_DELETE].SetEnable(m_bDeleteEnabled);

    if (!HasLiveCharacter())
    {
        g_CharMakeWin.Show(true);
    }
}

bool CCharSelMainWin::UpdateMouseEvent()
{
    if (!IsVisible())
        return true;

    // Was CWin::CursorInWin(WA_ALL) -- ported directly (see CServerSelWin's identical pattern).
    RECT rc;
    ::SetRect(&rc, m_ptPos.x, m_ptPos.y, m_ptPos.x + m_Size.cx, m_ptPos.y + m_Size.cy);
    if (::PtInRect(&rc, CInput::Instance().GetCursorPos()))
        return false;

    return true;
}

bool CCharSelMainWin::Update()
{
    if (!IsVisible())
        return true;

    // The actual fix for a real, reported bug -- see this class's header comment. Block button
    // click-state processing entirely while a modal overlay is open, since the legacy
    // CWin::m_bActive gate this used to rely on for the same purpose doesn't reliably deactivate
    // on a timely basis. Buttons still Update() below regardless (matching CServerSelWin -- CButton
    // self-gates on its own Show() flag), just their IsClick()/RmlClick* results go unconsumed.
    const bool modalOpen = IsCharacterSceneModalOpen();

    for (auto& button : m_aBtn)
        button.Update();

    if (modalOpen)
        return true;

    if (m_aBtn[CSMW_BTN_CONNECT].IsClick() || m_bRmlConnectClicked)
    {
        m_bRmlConnectClicked = false;
        ::StartGame();
    }
    else if (m_aBtn[CSMW_BTN_MENU].IsClick() || m_bRmlMenuClicked)
    {
        m_bRmlMenuClicked = false;
        g_SysMenuWin.Show(true);
    }
    else if (m_aBtn[CSMW_BTN_CREATE].IsClick() || m_bRmlCreateClicked)
    {
        m_bRmlCreateClicked = false;
        g_CharMakeWin.Show(true);
    }
    else if (m_aBtn[CSMW_BTN_DELETE].IsClick() || m_bRmlDeleteClicked)
    {
        m_bRmlDeleteClicked = false;
        DeleteCharacter();
    }

    return true;
}

bool CCharSelMainWin::Render()
{
    // RmlUi's #panel now owns 100% of this bar's visuals (buttons, info-bar background, deco
    // flourish, account-block message) in every theme -- see this class's header comment. The
    // legacy CSprites/CButtons stay alive purely for their geometry/click-detection bookkeeping,
    // never rendered; SyncRmlModel() is the only thing this override still needs to do.
    SyncRmlModel();
    return true;
}

void CCharSelMainWin::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto syncBool = [this](bool CharSelMainRmlModel::* field, const char* boundName, bool value)
    {
        if (m_RmlBinder.GetModel().*field != value)
        {
            m_RmlBinder.GetModel().*field = value;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    syncBool(&CharSelMainRmlModel::createDisabled, "create_disabled", !m_bCreateEnabled);
    syncBool(&CharSelMainRmlModel::connectDisabled, "connect_disabled", !m_bConnectEnabled);
    syncBool(&CharSelMainRmlModel::deleteDisabled, "delete_disabled", !m_bDeleteEnabled);
    syncBool(&CharSelMainRmlModel::accountBlockHidden, "account_block_hidden", !m_bAccountBlockItem);

    auto syncLabel = [this](Rml::String CharSelMainRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const std::string utf8 = StringUtils::WideToNarrow(text);
        if (m_RmlBinder.GetModel().*field != utf8)
        {
            m_RmlBinder.GetModel().*field = utf8;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    syncLabel(&CharSelMainRmlModel::accountBlockLine1, "account_block_line1", I18N::Game::ThisAccountIsItemBlocked);
    syncLabel(&CharSelMainRmlModel::accountBlockLine2, "account_block_line2", I18N::Game::PleaseCheckOnHttpMuonlineWebzenComSite);
}

void CCharSelMainWin::DeleteCharacter()
{
    CHARACTER* selected = GetSelectedCharacter();
    if (selected == nullptr)
        return;

    CSceneUICoordinator& uiManager = CSceneUICoordinator::Instance();

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
