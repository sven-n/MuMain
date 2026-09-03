//*****************************************************************************
// File: LoginMainWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/LoginMainWin.h"

#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include "Network/Server/WSclient.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <cmath>
#include <functional>

extern unsigned int WindowWidth, WindowHeight;

namespace
{
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

//=============================================================================
// Global Variables
//=============================================================================

//=============================================================================
// Constructor / Destructor
//=============================================================================

CLoginMainWin::CLoginMainWin() {}

CLoginMainWin::~CLoginMainWin() {}

//=============================================================================
// Public Methods
//=============================================================================

void CLoginMainWin::Create()
{
    for (int i = 0; i <= LMW_BTN_CREDIT; ++i)
        m_aBtn[i].Create(54, 30, BITMAP_LOG_IN + 4 + i, 3, 2, 1);

    // WindowWidth (ZzzOpenglUtil.cpp), not CInput::Instance().GetScreenWidth() -- same latent
    // staleness risk as the bug SetPosition()'s own comment documents fixing elsewhere; this call
    // predates that fix and was missed. #panel's width is pushed from this exact CWin::GetWidth()
    // value (below), and #btn_credit anchors right:0dp off #panel's own right edge, so a stale
    // width here would misplace/misclick that button specifically.
    CWin::Create(static_cast<int>(WindowWidth) - 30 * 2, m_aBtn[0].GetHeight(), -2);
    SetMovable(false);

    // Legacy CButtons stay registered for redundant click detection only -- RenderControls() is
    // no longer overridden, so CWin::RenderButtons() never runs for them; RmlUi renders 100% of
    // this bar's visuals in every theme (see the class header comment).
    for (int i = 0; i < LMW_BTN_MAX; ++i)
        CWin::RegisterButton(&m_aBtn[i]);

    // RmlUi migration, Batch 2 -- see this class's header comment. Guarded the same way
    // CLoginWin::Create() is (CUIMng::RepositionSceneUI() re-runs Create() on resolution change),
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
}

void CLoginMainWin::PreRelease()
{
    // CUIMng::RemoveWinList() (run on every scene transition) calls Release() on every window
    // in its list unconditionally -- CWin's own Release()/PreRelease() has no knowledge of
    // m_pRmlDoc, so without this it silently keeps rendering (still Shown, still in the
    // Context) on whatever scene comes next. Confirmed the hard way: transitioning from the
    // login scene to character-select left this window's Menu/Credit icons visibly overlapping
    // CharSelMainWin's own button bar, since nothing had ever told the RmlUi document to hide.
    // Hide(), not unload -- the document/model are meant to be created once and reused (see
    // Create()'s own guard comment), matching CLoginWin's precedent.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CLoginMainWin::SetPosition(int nXCoord, int nYCoord)
{
    CWin::SetPosition(nXCoord, nYCoord);

    // Same combined ratio RmlUi's own dp unit uses (RmlUiRuntime.cpp's ApplyUIScale(),
    // UIScalePercent x UI::Scaling::ViewportFitScale()) -- the legacy credit button's own
    // click-detection width must track login_main.rcss's now-dp .btn-icon width, or its hit rect
    // drifts from the RmlUi-rendered button at any UI scale/resolution other than the reference
    // case. Lower stakes than CharSelMainWin's IsCursorOnUI() case (this window's click handling
    // already treats the legacy CButton and RmlUi's own click listener as redundant -- see
    // UpdateWhileActive()'s `||`), but the same fix is cheap and correct to apply here too.
    //
    // UI::Scaling::CompanionRatio() (UITransform.cpp) is the single shared implementation of this
    // formula -- pass the WindowWidth/WindowHeight globals (ZzzOpenglUtil.cpp), not
    // CInput::Instance().GetScreenWidth()/GetScreenHeight(): a real, screenshot-confirmed bug in
    // LoginWin.cpp traced back to CInput's own copy of the screen size not reliably matching
    // WindowWidth/WindowHeight (the exact values RmlUiRuntime::OnResize() uses).
    const float uiScale = UI::Scaling::CompanionRatio(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    const int creditWidth = static_cast<int>(std::lround(m_aBtn[LMW_BTN_CREDIT].GetWidth() * uiScale));

    m_aBtn[LMW_BTN_MENU].SetPosition(nXCoord, nYCoord);
    m_aBtn[LMW_BTN_CREDIT].SetPosition(nXCoord + CWin::GetWidth() - creditWidth, nYCoord);

    // RmlUi panel: positioned/sized to the same real window-pixel geometry CWin's own bookkeeping
    // uses (see CLoginWin::SetPosition's identical comment) -- #panel's own bounding box is a
    // genuinely computed value (tied to screen size and this bar's placement relative to sibling
    // login-scene elements, layout-and-scaling.md's "genuine live computed result" carve-out), so
    // it stays C++-pushed. Its CHILDREN don't anymore, as of 2026-09-03: btn_menu/btn_credit
    // position themselves via login_main.rcss's anchor-left/right:0dp rules instead, picking up
    // the same dp auto-fit .btn-icon's own width/height already have.
    if (m_pRmlDoc)
    {
        if (Rml::Element* panel = m_pRmlDoc->GetElementById("panel"))
        {
            panel->SetProperty("left", std::to_string(nXCoord) + "px");
            panel->SetProperty("top", std::to_string(nYCoord) + "px");
            panel->SetProperty("width", std::to_string(CWin::GetWidth()) + "px");
            panel->SetProperty("height", std::to_string(CWin::GetHeight()) + "px");
        }
    }
}

void CLoginMainWin::Show(bool bShow)
{
    CWin::Show(bShow);

    for (int i = 0; i < LMW_BTN_MAX; ++i)
        m_aBtn[i].Show(bShow);

    if (m_pRmlDoc)
    {
        if (bShow) m_pRmlDoc->Show();
        else       m_pRmlDoc->Hide();
    }
}

void CLoginMainWin::UpdateWhileActive(double dDeltaTick)
{
    // RmlUi-triggered clicks no longer come through here -- see RmlClickMenu()/RmlClickCredit()'s
    // header comment. This is now only the legacy CButton companion's own (still m_bActive-gated)
    // click-detection path.
    if (m_aBtn[LMW_BTN_MENU].IsClick())
        OpenSysMenu();
    else if (m_aBtn[LMW_BTN_CREDIT].IsClick())
        OpenCredits();
}

void CLoginMainWin::OpenSysMenu()
{
    CUIMng& rUIMng = CUIMng::Instance();
    rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
    rUIMng.SetSysMenuWinShow(true);
}

void CLoginMainWin::OpenCredits()
{
    CUIMng& rUIMng = CUIMng::Instance();
    SocketClient->ToConnectServer()->SendServerListRequest();

    rUIMng.ShowWin(&rUIMng.m_CreditWin);

    ::StopMp3(MUSIC_MAIN_THEME);
    ::PlayMp3(MUSIC_MUTHEME);
}
