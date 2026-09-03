//*****************************************************************************
// File: SysMenuWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/SysMenuWin.h"
#include "I18N/All.h"

#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Scenes/SceneCore.h"

#include "Audio/DSPlaySound.h"
#include "UI/NewUI/NewUISystem.h"

#include "Network/Server/WSclient.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include "Core/Utilities/Log/muConsoleDebug.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>

#define	SMW_BTN_GAP		4

extern EGameScene  SceneFlag;
extern bool LogOut;

CSysMenuWin::CSysMenuWin()
{
}

CSysMenuWin::~CSysMenuWin()
{
}

void CSysMenuWin::Create()
{
    CInput rInput = CInput::Instance();
    // RmlUi migration, Batch 2: -2 (was the default -1) -- see this class's header comment.
    CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight(), -2);
    SetMovable(false);

    SImgInfo aiiBack[WE_BG_MAX] =
    {
        { BITMAP_SYS_WIN, 0, 0, 128, 128 },
        { BITMAP_SYS_WIN + 1, 0, 0, 213, 64 },
        { BITMAP_SYS_WIN + 2, 0, 0, 213, 43 },
        { BITMAP_SYS_WIN + 3, 0, 0, 5, 8 },
        { BITMAP_SYS_WIN + 4, 0, 0, 5, 8 }
    };
    m_winBack.Create(aiiBack, 1, 10);

    const wchar_t* apszBtnText[SMW_BTN_MAX] =
    { I18N::Game::ExitGame, I18N::Game::SelectServer, I18N::Game::Option385, I18N::Game::Close388 };
    DWORD adwBtnClr[4] =
    { CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0 };
    for (int i = 0; i < SMW_BTN_MAX; ++i)
    {
        m_aBtn[i].Create(108, 30, BITMAP_TEXT_BTN, 4, 2, 1);
        m_aBtn[i].SetText(apszBtnText[i], adwBtnClr);
        CWin::RegisterButton(&m_aBtn[i]);
    }

    switch (SceneFlag)
    {
    case LOG_IN_SCENE:
        m_aBtn[SMW_BTN_SERVER_SEL].SetEnable(false);
        m_winBack.SetLine(6);
        break;
    case CHARACTER_SCENE:
        m_aBtn[SMW_BTN_SERVER_SEL].SetEnable(true);
        m_winBack.SetLine(10);
        break;
    }
    m_bSelectServerEnabled = (SceneFlag == CHARACTER_SCENE);

    // RmlUi migration, Batch 2 -- see this class's header comment. Guarded the same way
    // CLoginWin::Create() is (CUIMng::RepositionSceneUI() re-runs Create() on resolution change).
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "sys_menu",
            [this](Rml::DataModelConstructor& c, SysMenuRmlModel& model)
            {
                c.Bind("select_server_hidden", &model.selectServerHidden);
                c.Bind("exit_game_label", &model.exitGameLabel);
                c.Bind("select_server_label", &model.selectServerLabel);
                c.Bind("option_label", &model.optionLabel);
                c.Bind("close_label", &model.closeLabel);

                c.BindEventCallback("sysmenu_exit_game_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickExitGame(); });
                c.BindEventCallback("sysmenu_select_server_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickSelectServer(); });
                c.BindEventCallback("sysmenu_option_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickOption(); });
                c.BindEventCallback("sysmenu_close_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickClose(); });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/sys_menu.rml");
    }

    SetPosition((rInput.GetScreenWidth() - m_winBack.GetWidth()) / 2,
        (rInput.GetScreenHeight() - m_winBack.GetHeight()) / 2);
}

void CSysMenuWin::PreRelease()
{
    m_winBack.Release();

    // See CLoginMainWin::PreRelease()'s identical comment -- CUIMng::RemoveWinList() Release()s
    // every window on every scene transition, and CWin's own Release() has no knowledge of
    // m_pRmlDoc, so without this it can keep rendering into whatever scene comes next if this
    // window happened to be open at the moment of transition.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CSysMenuWin::SetPosition(int nXCoord, int nYCoord)
{
    m_winBack.SetPosition(nXCoord, nYCoord);

    int nBtnPosX = m_winBack.GetXPos() + (m_winBack.GetWidth() - m_aBtn[0].GetWidth()) / 2;
    int nBtnGap = SMW_BTN_GAP + m_aBtn[0].GetHeight();
    int nBtnPosBaseTop = m_winBack.GetYPos() + 33;
    for (int i = 0; i < SMW_BTN_OPTION; ++i)
        m_aBtn[i].SetPosition(nBtnPosX, nBtnPosBaseTop + i * nBtnGap);

    int nCloseBtnPosY = m_winBack.GetYPos() + m_winBack.GetHeight() - 52;
    m_aBtn[SMW_BTN_CLOSE].SetPosition(nBtnPosX, nCloseBtnPosY);
    m_aBtn[SMW_BTN_OPTION].SetPosition(nBtnPosX, nCloseBtnPosY - nBtnGap);

    // 2026-09-03: no longer pushes anything to the RmlUi elements -- #panel centers itself via
    // base.rcss's .center-both utility class and sizes itself via a fixed dp width/height plus a
    // .compact modifier for the shorter login-scene variant (sys_menu.rml/.rcss, both themes);
    // every button is a fixed dp offset from the panel's own edges. See sys_menu.rcss's own
    // comment for the full derivation from this function's old math. The legacy m_winBack/m_aBtn
    // positioning above is unchanged -- still real screen-pixel geometry, still needed for their
    // own click-detection redundancy (UpdateWhileActive()'s `||`).
}
void CSysMenuWin::Show(bool bShow)
{
    CWin::Show(bShow);

    m_winBack.Show(bShow);
    for (int i = 0; i < SMW_BTN_MAX; ++i)
        m_aBtn[i].Show(bShow);

    if (m_pRmlDoc)
    {
        if (bShow) { SyncRmlModel(); m_pRmlDoc->Show(); }
        else       m_pRmlDoc->Hide();
    }
}

void CSysMenuWin::UpdateWhileActive(double dDeltaTick)
{
    if (m_aBtn[SMW_BTN_GAME_END].IsClick() || m_bRmlExitGameClicked)
    {
        m_bRmlExitGameClicked = false;
        CUIMng::Instance().PopUpMsgWin(MESSAGE_GAME_END_COUNTDOWN);
    }
    else if ((m_aBtn[SMW_BTN_SERVER_SEL].IsClick() || m_bRmlSelectServerClicked) && m_bSelectServerEnabled)
    {
        m_bRmlSelectServerClicked = false;
        g_ErrorReport.Write(L"> Menu - Join another server.");
        g_ErrorReport.WriteCurrentTime();
        LogOut = true;
        SocketClient->ToGameServer()->SendLogOut(LogOutType::BackToServerSelection);
        g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 2");

        CUIMng& rUIMng = CUIMng::Instance();
        rUIMng.HideWin(this);
        rUIMng.HideWin(&rUIMng.m_CharSelMainWin);
    }
    else if (m_aBtn[SMW_BTN_OPTION].IsClick() || m_bRmlOptionClicked)
    {
        m_bRmlOptionClicked = false;
        CUIMng& rUIMng = CUIMng::Instance();
        rUIMng.HideWin(this);
        g_pNewUISystem->Show(SEASON3B::INTERFACE_OPTION);
    }
    else if (m_aBtn[SMW_BTN_CLOSE].IsClick() || m_bRmlCloseClicked)
    {
        m_bRmlCloseClicked = false;
        CUIMng::Instance().HideWin(this);
    }
    else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
    {
        // ESC toggle is handled by CUIMng::Update()
        // No action needed here — CUIMng already hid this window
    }
}

void CSysMenuWin::RenderControls()
{
    // m_winBack no longer renders -- RmlUi's #backdrop/#panel own 100% of this window's visuals
    // (see this class's header comment). CWin::RenderButtons() also draws nothing visible since
    // the legacy CButtons are unregistered from any bitmap/text draw path once RmlUi renders
    // their labels; kept registered purely for redundant click detection like CLoginWin's.
    SyncRmlModel();
}

void CSysMenuWin::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    if (m_RmlBinder.GetModel().selectServerHidden != !m_bSelectServerEnabled)
    {
        m_RmlBinder.GetModel().selectServerHidden = !m_bSelectServerEnabled;
        m_RmlBinder.MarkDirty("select_server_hidden");
    }

    auto syncLabel = [this](Rml::String SysMenuRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const std::string utf8 = StringUtils::WideToNarrow(text);
        if (m_RmlBinder.GetModel().*field != utf8)
        {
            m_RmlBinder.GetModel().*field = utf8;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    syncLabel(&SysMenuRmlModel::exitGameLabel, "exit_game_label", I18N::Game::ExitGame);
    syncLabel(&SysMenuRmlModel::selectServerLabel, "select_server_label", I18N::Game::SelectServer);
    syncLabel(&SysMenuRmlModel::optionLabel, "option_label", I18N::Game::Option385);
    syncLabel(&SysMenuRmlModel::closeLabel, "close_label", I18N::Game::Close388);
}