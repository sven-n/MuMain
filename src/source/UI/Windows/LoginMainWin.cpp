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
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <functional>

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

CLoginMainWin::CLoginMainWin()
{
}

CLoginMainWin::~CLoginMainWin()
{
}

//=============================================================================
// Public Methods
//=============================================================================

void CLoginMainWin::Create()
{
    for (int i = 0; i <= LMW_BTN_CREDIT; ++i)
        m_aBtn[i].Create(54, 30, BITMAP_LOG_IN + 4 + i, 3, 2, 1);

    CWin::Create(
        CInput::Instance().GetScreenWidth() - 30 * 2,
        m_aBtn[0].GetHeight(),
        -2
    );
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

    m_aBtn[LMW_BTN_MENU].SetPosition(nXCoord, nYCoord);

    m_aBtn[LMW_BTN_CREDIT].SetPosition(
        nXCoord + CWin::GetWidth() - m_aBtn[LMW_BTN_CREDIT].GetWidth(),
        nYCoord
    );

    // RmlUi panel: positioned at the same real window-pixel origin CWin's own bookkeeping uses
    // (see CLoginWin::SetPosition's identical comment). btn_credit's left is pushed separately
    // (not a fixed RCSS offset like btn_menu) because it's screen-width-dependent, matching the
    // legacy CButton positioning math right above.
    if (m_pRmlDoc)
    {
        if (Rml::Element* panel = m_pRmlDoc->GetElementById("panel"))
        {
            panel->SetProperty("left", std::to_string(nXCoord) + "px");
            panel->SetProperty("top", std::to_string(nYCoord) + "px");
            panel->SetProperty("width", std::to_string(CWin::GetWidth()) + "px");
            panel->SetProperty("height", std::to_string(CWin::GetHeight()) + "px");
        }
        if (Rml::Element* credit = m_pRmlDoc->GetElementById("btn_credit"))
            credit->SetProperty("left", std::to_string(CWin::GetWidth() - m_aBtn[LMW_BTN_CREDIT].GetWidth()) + "px");
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
    CUIMng& rUIMng = CUIMng::Instance();

    if (m_aBtn[LMW_BTN_MENU].IsClick() || m_bRmlMenuClicked)
    {
        m_bRmlMenuClicked = false;
        rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
        rUIMng.SetSysMenuWinShow(true);
    }
    else if (m_aBtn[LMW_BTN_CREDIT].IsClick() || m_bRmlCreditClicked)
    {
        m_bRmlCreditClicked = false;
        SocketClient->ToConnectServer()->SendServerListRequest();

        rUIMng.ShowWin(&rUIMng.m_CreditWin);

        ::StopMp3(MUSIC_MAIN_THEME);
        ::PlayMp3(MUSIC_MUTHEME);
    }
}
