//*****************************************************************************
// File: LoginMainWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/LoginMainWin.h"

#include "Core/Input/Input.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Windows/SysMenuWin.h"
#include "UI/Windows/CreditWin.h"
#include "Network/Server/WSclient.h"
#include "Core/Globals/_enum.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlNativeText.h"
#include "UI/RmlBridge/RmlTheme.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <cmath>
#include <functional>

extern unsigned int WindowWidth, WindowHeight;

namespace
{
    // This bar's own bounding-box height, used only for the click-gate rect below.
    constexpr int kBtnHeight = 30;

    // The buttons' height in login_main.rcss's units (px, or the legacy theme's scene-window rem).
    int ButtonBarHeight()
    {
        return static_cast<int>(
            std::lround(kBtnHeight * UI::RmlBridge::SceneWindowPixelRatio(static_cast<int>(WindowWidth),
                                                                          static_cast<int>(WindowHeight))));
    }

    // Self-owning click->callback listener; simpler than an RmlModelBinder for two click slots.
    class ClickCallbackListener : public Rml::EventListener
    {
    public:
        explicit ClickCallbackListener(std::function<void()> callback) : m_Callback(std::move(callback)) {}
        void ProcessEvent(Rml::Event&) override { m_Callback(); }
        void OnDetach(Rml::Element*) override { delete this; }
    private:
        std::function<void()> m_Callback;
    };
}

CLoginMainWin g_LoginMainWin;

CLoginMainWin::CLoginMainWin() {}

CLoginMainWin::~CLoginMainWin()
{
    Release();
}

void CLoginMainWin::Create()
{
    Release();

    // Reads WindowWidth, not CInput::Instance().GetScreenWidth() -- the latter can go stale and
    // misplace #btn_credit, which anchors off #panel's right edge using this exact value.
    m_Size.cx = static_cast<int>(WindowWidth) - 30 * 2;
    m_Size.cy = ButtonBarHeight();
    m_ptPos.x = m_ptPos.y = 0;

    // Builds once, and is only repositioned and resized afterward.
    m_RmlView.Ensure();

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_LOGIN_MAIN, this);
    Show(false);
}

void CLoginMainWin::OnRmlBuilt()
{
    Rml::ElementDocument* document = m_RmlView.Document();
    if (Rml::Element* e = document->GetElementById("btn_menu"))
        e->AddEventListener(Rml::EventId::Click, new ClickCallbackListener([this] { RmlClickMenu(); }));
    if (Rml::Element* e = document->GetElementById("btn_credit"))
        e->AddEventListener(Rml::EventId::Click, new ClickCallbackListener([this] { RmlClickCredit(); }));
}

void CLoginMainWin::OnRmlReloaded()
{
    // Bottom-anchored (CSceneUICoordinator::CreateLoginScene()): the bar's height follows the theme.
    const int previousHeight = m_Size.cy;
    m_Size.cy = ButtonBarHeight();
    SetPosition(m_ptPos.x, m_ptPos.y + previousHeight - m_Size.cy);
}

void CLoginMainWin::Release()
{
    // Called explicitly at each scene transition; without it this bar's icons stay visible and
    // overlap the next scene's own UI. Hide, not unload -- the document is created once and reused.
    m_RmlView.Hide();
}

void CLoginMainWin::SetPosition(int nXCoord, int nYCoord)
{
    m_ptPos.x = nXCoord;
    m_ptPos.y = nYCoord;

    // #panel's bounding box is a genuinely computed value (tied to screen size), so it stays
    // C++-pushed; its children position themselves via login_main.rcss's anchor rules instead.
    if (m_RmlView.Document())
    {
        if (Rml::Element* panel = m_RmlView.Document()->GetElementById("panel"))
        {
            panel->SetProperty("left", std::to_string(nXCoord) + "px");
            panel->SetProperty("top", std::to_string(nYCoord) + "px");
            panel->SetProperty("width", std::to_string(m_Size.cx) + "px");
            panel->SetProperty("height", std::to_string(m_Size.cy) + "px");
        }
    }
}

void CLoginMainWin::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);

    if (m_RmlView.Document())
    {
        if (bShow) m_RmlView.Document()->Show();
        else       m_RmlView.Document()->Hide();
    }
}

bool CLoginMainWin::UpdateMouseEvent()
{
    if (!IsVisible())
        return true;

    RECT rc;
    ::SetRect(&rc, m_ptPos.x, m_ptPos.y, m_ptPos.x + m_Size.cx, m_ptPos.y + m_Size.cy);
    if (::PtInRect(&rc, CInput::Instance().GetCursorPos()))
        return false;

    return true;
}

void CLoginMainWin::OpenSysMenu()
{
    g_SysMenuWin.Show(true);
}

void CLoginMainWin::OpenCredits()
{
    SocketClient->ToConnectServer()->SendServerListRequest();

    g_CreditWin.Show(true);

    ::StopMp3(MUSIC_MAIN_THEME);
    ::PlayMp3(MUSIC_MUTHEME);
}
