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
    // This bar's own bounding-box height, used only for the click-gate rect below -- RmlUi
    // positions/sizes the visible buttons itself via login_main.rcss's anchor rules.
    constexpr int kBtnHeight = 30;

    // Mirrors UI::RmlBridge::RmlDraggable.cpp's self-owning listener pattern -- this window has
    // no dynamic state to bind (see this class's header comment), so a plain click->callback
    // listener is simpler than standing up an RmlModelBinder just for two BindEventCallback slots.
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

    // WindowWidth (ZzzOpenglUtil.cpp), not CInput::Instance().GetScreenWidth() -- same latent
    // staleness risk as the bug SetPosition()'s own comment documents fixing elsewhere; this call
    // predates that fix and was missed. #panel's width is pushed from this exact value (below),
    // and #btn_credit anchors right:0dp off #panel's own right edge, so a stale width here would
    // misplace/misclick that button specifically.
    m_Size.cx = static_cast<int>(WindowWidth) - 30 * 2;
    m_Size.cy = kBtnHeight;
    m_ptPos.x = m_ptPos.y = 0;

    // RmlUi migration, Batch 2 -- see this class's header comment. Guarded the same way
    // CLoginWin::Create() is (CSceneUICoordinator::RepositionSceneUI() re-runs Create() on resolution change),
    // so the document is loaded once, ever, and only repositioned/resized afterward.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
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

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_LOGIN_MAIN, this);
    Show(false);
}

void CLoginMainWin::Release()
{
    // CUIMng::RemoveWinList() (run on every scene transition) used to call Release() on every
    // window in its list unconditionally, before this window migrated off it -- called explicitly
    // now at the same call sites. CWin's own Release()/PreRelease() had no knowledge of m_pRmlDoc,
    // so without this it silently kept rendering (still Shown, still in the Context) on whatever
    // scene comes next. Confirmed the hard way: transitioning from the login scene to
    // character-select left this window's Menu/Credit icons visibly overlapping CharSelMainWin's
    // own button bar, since nothing had ever told the RmlUi document to hide. Hide(), not unload
    // -- the document/model are meant to be created once and reused (see Create()'s own guard
    // comment), matching CLoginWin's precedent.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CLoginMainWin::SetPosition(int nXCoord, int nYCoord)
{
    m_ptPos.x = nXCoord;
    m_ptPos.y = nYCoord;

    // RmlUi panel: positioned/sized to the same real window-pixel geometry this window's own
    // bookkeeping uses (see CLoginWin::SetPosition's identical comment) -- #panel's own bounding
    // box is a genuinely computed value (tied to screen size and this bar's placement relative to
    // sibling login-scene elements -- a genuine live computed result, not a static value), so it
    // stays C++-pushed. Its CHILDREN don't:
    // btn_menu/btn_credit position themselves via login_main.rcss's anchor-left/right:0dp rules
    // instead, picking up the same dp auto-fit .btn-icon's own width/height already have.
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

    // Was CWin::CursorInWin(WA_ALL) -- ported directly (see CServerSelWin's identical pattern).
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
