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
#include "UI/RmlBridge/RmlTheme.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <functional>

extern unsigned int WindowWidth, WindowHeight;

namespace
{
    // This bar's own bounding-box height, used only for the click-gate rect below.
    constexpr int kBtnHeight = 30;

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
    m_Size.cy = kBtnHeight;
    m_ptPos.x = m_ptPos.y = 0;

    // Guarded so the document is loaded once, ever, and only repositioned/resized afterward.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_LOGIN_MAIN, this);
    Show(false);
}

void CLoginMainWin::BuildRmlUi()
{
    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/login_main.rml");
    if (m_pRmlDoc)
    {
        if (Rml::Element* e = m_pRmlDoc->GetElementById("btn_menu"))
            e->AddEventListener(Rml::EventId::Click, new ClickCallbackListener([this] { RmlClickMenu(); }));
        if (Rml::Element* e = m_pRmlDoc->GetElementById("btn_credit"))
            e->AddEventListener(Rml::EventId::Click, new ClickCallbackListener([this] { RmlClickCredit(); }));
    }
}

void CLoginMainWin::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    const bool wasVisible = m_pRmlDoc->IsVisible();
    RmlUiRuntime::Instance().GetContext()->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    SetPosition(m_ptPos.x, m_ptPos.y);
    if (m_pRmlDoc) { if (wasVisible) m_pRmlDoc->Show(); else m_pRmlDoc->Hide(); }
}

void CLoginMainWin::Release()
{
    // Called explicitly at each scene transition; without it this bar's icons stay visible and
    // overlap the next scene's own UI. Hide, not unload -- the document is created once and reused.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CLoginMainWin::SetPosition(int nXCoord, int nYCoord)
{
    m_ptPos.x = nXCoord;
    m_ptPos.y = nYCoord;

    // #panel's bounding box is a genuinely computed value (tied to screen size), so it stays
    // C++-pushed; its children position themselves via login_main.rcss's anchor rules instead.
    if (m_pRmlDoc)
    {
        if (Rml::Element* panel = m_pRmlDoc->GetElementById("panel"))
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

    if (m_pRmlDoc)
    {
        if (bShow) m_pRmlDoc->Show();
        else       m_pRmlDoc->Hide();
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
