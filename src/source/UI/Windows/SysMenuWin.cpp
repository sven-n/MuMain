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

    // Guarded so BuildRmlUi() runs once; Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
        BuildRmlUi();

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_SYS_MENU, this);

    Show(false);
}

void CSysMenuWin::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "sys_menu",
        [this](Rml::DataModelConstructor& c, SysMenuRmlModel& model)
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
        });

    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/sys_menu.rml");
}

void CSysMenuWin::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    // See CLoginWin::ReloadRmlTheme()'s comment on why this reads m_pRmlDoc directly.
    const bool wasVisible = m_pRmlDoc->IsVisible();
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    if (wasVisible) { SyncRmlModel(); if (m_pRmlDoc) { m_pRmlDoc->PullToFront(); m_pRmlDoc->Show(); } }
}

void CSysMenuWin::Release()
{
    // Called explicitly at each scene transition; no base-class auto-release for m_pRmlDoc.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CSysMenuWin::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);

    if (m_pRmlDoc)
    {
        // PullToFront() is required: this document is never recreated after scene setup, so it
        // otherwise stays at its original creation-time z-order (GetLayerDepth() only orders this
        // legacy manager's own dispatch, not RmlUi's document stack).
        if (bShow) { SyncRmlModel(); m_pRmlDoc->PullToFront(); m_pRmlDoc->Show(); }
        else       m_pRmlDoc->Hide();
    }
}

bool CSysMenuWin::Update()
{
    // ESC toggle is handled by CSceneUICoordinator::Update() -- no action needed here.
    return true;
}

void CSysMenuWin::ExitGame()
{
    CSceneUICoordinator::Instance().PopUpMsgWin(MESSAGE_GAME_END_COUNTDOWN);
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
    syncLabel(&SysMenuRmlModel::systemMenuLabel, "system_menu_label", I18N::Game::SystemMenu);
    syncLabel(&SysMenuRmlModel::exitGameLabel, "exit_game_label", I18N::Game::ExitGame);
    syncLabel(&SysMenuRmlModel::selectServerLabel, "select_server_label", I18N::Game::SelectServer);
    syncLabel(&SysMenuRmlModel::optionLabel, "option_label", I18N::Game::Option385);
    syncLabel(&SysMenuRmlModel::closeLabel, "close_label", I18N::Game::Close388);
}
