//*****************************************************************************
// File: LoginMainWin.h
//*****************************************************************************

#if !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
#define AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_

#pragma once

#include "UI/Widgets/Win.h"
#include "UI/Widgets/Button.h"

#define LMW_BTN_MENU 0
#define LMW_BTN_CREDIT 1
#define LMW_BTN_MAX 2

namespace Rml { class ElementDocument; }

// RmlUi migration, Batch 2 (see the login/character-select scope in the migration plan): this
// window's two buttons are pure image buttons with no I18N text and no dynamic state, so unlike
// CLoginWin/CSysMenuWin/COptionWin it needs no UI::RmlBridge::RmlModelBinder -- click detection is
// wired directly via Rml::Element::AddEventListener, same idiom UI::RmlBridge::RmlDraggable.cpp
// already uses for its own self-owning listener. The legacy CButtons stay registered (redundant,
// harmless detection path); RmlUi renders 100% of this bar's visuals in every theme.
class CLoginMainWin : public CWin
{
protected:
    CButton m_aBtn[LMW_BTN_MAX];

public:
    CLoginMainWin();
    virtual ~CLoginMainWin();

    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);

    // Invoked from the RmlUi document's click listeners (see Create()) -- act immediately rather
    // than deferring to UpdateWhileActive() via a polled-and-cleared flag (the shape this used to
    // mirror, same as CLoginWin::RmlClickOk()). 2026-09-03: found, via a real reproduction (a
    // credit-button click that visibly did nothing, followed later by an unrelated click on the
    // menu button that opened the *credits* window instead of the menu -- proof the credit click
    // itself had registered and just sat unconsumed), that UpdateWhileActive() can go many frames
    // without running at all: it's gated behind CWin::m_bActive, driven by CUIMng's legacy
    // click-activation system (list-order hit-testing, deferred one-frame activation -- see
    // UIMng.cpp/STATUS.md's "Findings worth knowing" entry), which is a fundamentally less
    // reliable signal than RmlUi's own click event -- this listener already firing IS proof the
    // click happened, with no need to wait for anything else to agree. Safe to call CUIMng
    // methods (ShowWin, list mutation) directly here: this fires from
    // RmlUiRuntime::ProcessSdlEvent(), called from Winmain.cpp's SDL event pump, which always
    // completes before CUIMng::Update() runs later the same frame -- no concurrent list iteration.
    void RmlClickMenu() { OpenSysMenu(); }
    void RmlClickCredit() { OpenCredits(); }

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);

private:
    // Shared by the immediate RmlUi callbacks above and UpdateWhileActive()'s legacy
    // m_aBtn[...].IsClick() polling (the redundant CButton companion's own click path -- see this
    // class's header comment on why the legacy CButtons stay registered).
    void OpenSysMenu();
    void OpenCredits();

    Rml::ElementDocument* m_pRmlDoc = nullptr;
};

#endif // !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
