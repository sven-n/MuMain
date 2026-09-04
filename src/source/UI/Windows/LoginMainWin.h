//*****************************************************************************
// File: LoginMainWin.h
//*****************************************************************************

#if !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
#define AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_

#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/Widgets/Button.h"

#define LMW_BTN_MENU 0
#define LMW_BTN_CREDIT 1
#define LMW_BTN_MAX 2

namespace Rml { class ElementDocument; }

// RmlUi migration, Batch 2 (see the login/character-select scope in the migration plan): this
// window's two buttons are pure image buttons with no I18N text and no dynamic state, so unlike
// CLoginWin/CSysMenuWin it needs no UI::RmlBridge::RmlModelBinder -- click detection is
// wired directly via Rml::Element::AddEventListener, same idiom UI::RmlBridge::RmlDraggable.cpp
// already uses for its own self-owning listener. The legacy CButtons stay registered (redundant,
// harmless detection path); RmlUi renders 100% of this bar's visuals in every theme.
//
// CUIMng/CNewUIManager merger (docs/newui-legacy-merger.md) Phase 2: migrated off CWin onto
// SEASON3B::CNewUIObj -- the last of the login/character-scene CWin windows besides CLoginWin
// itself (Phase 3). Not modal -- UpdateMouseEvent() claims only within its own bounding rect
// (CServerSelWin's established pattern), matching this bar's small footprint. This migration also
// permanently retires the last live consequence of the m_LoginWin/m_LoginMainWin hit-test-overlap
// bug CreateLoginScene()'s own comment documents (RmlClickMenu()/RmlClickCredit() already bypassed
// it; the legacy CButton companion's click path no longer can be starved by m_LoginWin's own
// list-order priority now that this window's dispatch runs before any legacy m_WinList walk at
// all, regardless of overlap).
class CLoginMainWin : public SEASON3B::CNewUIObj
{
protected:
    CButton m_aBtn[LMW_BTN_MAX];

    // Replaces CWin::m_ptPos/m_Size -- no shared rect facility on the CNewUIObj side (matching
    // every pre-existing CNewUIObj window), so this window keeps its own bounding box, same as
    // CServerSelWin's established pattern.
    POINT m_ptPos;
    SIZE m_Size;

public:
    CLoginMainWin();
    ~CLoginMainWin() override;

    void Create();
    void Release(); // was CWin::PreRelease() (an override hook CWin::Release() called
                     // automatically) -- called explicitly now, same as CCreditWin's own Release().
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow) override;
    int GetWidth() const
    {
        return m_Size.cx;
    }
    int GetHeight() const
    {
        return m_Size.cy;
    }

    // Invoked from the RmlUi document's click listeners (see Create()) -- act immediately rather
    // than deferring to Update() via a polled-and-cleared flag (the shape this used to mirror,
    // same as CLoginWin::RmlClickOk()). 2026-09-03: found, via a real reproduction (a
    // credit-button click that visibly did nothing, followed later by an unrelated click on the
    // menu button that opened the *credits* window instead of the menu -- proof the credit click
    // itself had registered and just sat unconsumed), that UpdateWhileActive() (this window's own
    // Update() override now) can go many frames without running at all under the legacy
    // activation system this window has since migrated off of. This listener already firing IS
    // proof the click happened, with no need to wait for anything else to agree.
    void RmlClickMenu() { OpenSysMenu(); }
    void RmlClickCredit() { OpenCredits(); }

    // SEASON3B::INewUIBase
    bool Render() override
    {
        return true;
    }
    bool Update() override;
    // Was CWin::CursorInWin(WA_ALL) -- claims any click within its own bounding box, ported from
    // CServerSelWin's established pattern. Not modal.
    bool UpdateMouseEvent() override;
    bool UpdateKeyEvent() override
    {
        return true;
    }
    float GetLayerDepth() override
    {
        return 15.0f;
    }

private:
    // Shared by the immediate RmlUi callbacks above and Update()'s legacy m_aBtn[...].IsClick()
    // polling (the redundant CButton companion's own click path -- see this class's header
    // comment on why the legacy CButtons stay registered).
    void OpenSysMenu();
    void OpenCredits();

    Rml::ElementDocument* m_pRmlDoc = nullptr;
};

// Replaces CUIMng's old `CLoginMainWin m_LoginMainWin;` member, same convention as g_CreditWin.
extern CLoginMainWin g_LoginMainWin;

#endif // !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
