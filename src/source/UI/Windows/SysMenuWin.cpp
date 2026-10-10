//*****************************************************************************
// File: SysMenuWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/SysMenuWin.h"
#include "I18N/All.h"

#include "UI/Core/SceneUICoordinator.h"
#include "Character/CharSelMainWin.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Scenes/SceneCore.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"

#include "Network/Server/WSclient.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include "Core/Utilities/Log/muConsoleDebug.h"
#include "Core/Globals/_enum.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>

extern EGameScene  SceneFlag;
extern bool LogOut;

CSysMenuWin g_SysMenuWin;

CSysMenuWin::CSysMenuWin()
{
}

CSysMenuWin::~CSysMenuWin()
{
    Release();
}

void CSysMenuWin::Create()
{
    Release();

    m_bSelectServerEnabled = (SceneFlag == CHARACTER_SCENE);

    // Builds once; Create() re-runs on resolution change.
    m_RmlView.Ensure();

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_SYS_MENU, this);

    Show(false);
}

void CSysMenuWin::BindRmlModel(Rml::DataModelConstructor& c, SysMenuRmlModel& model)
{
    c.Bind("select_server_hidden", &model.selectServerHidden);
    c.Bind("system_menu_label", &model.systemMenuLabel);
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
}

void CSysMenuWin::Release()
{
    // Called explicitly at each scene transition; no base-class auto-release for m_RmlView.Document().
    m_RmlView.Hide();

    // Base-class visibility reset (same fix as CServerMsgWin::Release()/CMsgWin::Release()) -- ESC
    // right before entering the game leaves this open at the exact transition instant otherwise,
    // stranding IsVisible() at true for whatever later code checks it during MAIN_SCENE.
    mu::ui::window::CObject::Show(false);
}

void CSysMenuWin::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);

    if (m_RmlView.Document())
    {
        // PullToFront() is required: this document is never recreated after scene setup, so it
        // otherwise stays at its original creation-time z-order (GetLayerDepth() only orders this
        // legacy manager's own dispatch, not RmlUi's document stack).
        if (bShow) { SyncRmlModel(); m_RmlView.Document()->PullToFront(); m_RmlView.Document()->Show(); }
        else       m_RmlView.Document()->Hide();
    }
}

bool CSysMenuWin::Update()
{
    // ESC toggle is handled by CSceneUICoordinator::Update() -- no action needed here.
    return true;
}

// Closes the game at once, as the world's menu does.
void CSysMenuWin::ExitGame()
{
    g_ErrorReport.Write(L"> Menu - Exit game.");
    g_ErrorReport.WriteCurrentTime();
    ::PostMessage(g_hWnd, WM_CLOSE, 0, 0);
}

void CSysMenuWin::SelectServer()
{
    g_ErrorReport.Write(L"> Menu - Join another server.");
    g_ErrorReport.WriteCurrentTime();
    LogOut = true;
    SocketClient->ToGameServer()->SendLogOut(LogOutType::BackToServerSelection);
    g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 2");

    Show(false);
    g_CharSelMainWin.Show(false);
}

void CSysMenuWin::OpenOptions()
{
    Show(false);
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_OPTION);
}

void CSysMenuWin::Close()
{
    Show(false);
}

bool CSysMenuWin::Render()
{
    SyncRmlModel();
    return true;
}

void CSysMenuWin::SyncRmlModel()
{
    if (!m_RmlView.Document()) return;

    if (m_RmlView.GetModel().selectServerHidden != !m_bSelectServerEnabled)
    {
        m_RmlView.GetModel().selectServerHidden = !m_bSelectServerEnabled;
        m_RmlView.MarkDirty("select_server_hidden");
    }

    auto syncLabel = [this](Rml::String SysMenuRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const std::string utf8 = StringUtils::WideToNarrow(text);
        if (m_RmlView.GetModel().*field != utf8)
        {
            m_RmlView.GetModel().*field = utf8;
            m_RmlView.MarkDirty(boundName);
        }
    };
    syncLabel(&SysMenuRmlModel::systemMenuLabel, "system_menu_label", I18N::Game::SystemMenu);
    syncLabel(&SysMenuRmlModel::exitGameLabel, "exit_game_label", I18N::Game::ExitGame);
    syncLabel(&SysMenuRmlModel::selectServerLabel, "select_server_label", I18N::Game::SelectServer);
    syncLabel(&SysMenuRmlModel::optionLabel, "option_label", I18N::Game::Option385);
    syncLabel(&SysMenuRmlModel::closeLabel, "close_label", I18N::Game::Close388);
}
