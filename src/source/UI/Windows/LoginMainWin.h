//*****************************************************************************
// File: LoginMainWin.h
//*****************************************************************************

#if !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
#define AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"

namespace Rml { class ElementDocument; }

// This window's two buttons are pure image buttons with no I18N text and no dynamic state, so unlike
// CLoginWin/CSysMenuWin it needs no UI::RmlBridge::RmlModelBinder -- click detection is
// wired directly via Rml::Element::AddEventListener, same idiom UI::RmlBridge::RmlDraggable.cpp
// already uses for its own self-owning listener. RmlUi renders 100% of this bar's visuals and owns
// click detection in every theme.
//
// Migrated off CWin onto mu::ui::window::CObject. Not modal -- UpdateMouseEvent() claims only
// within its own bounding rect (CServerSelWin's established pattern), matching this bar's small
// footprint. See CreateLoginScene()'s own comment for the g_LoginWin/g_LoginMainWin
// hit-test-overlap bug this migration closed out.
class CLoginMainWin : public mu::ui::window::CObject
{
protected:
    // Replaces CWin::m_ptPos/m_Size -- no shared rect facility on the CObject side (matching
    // every pre-existing CObject window), so this window keeps its own bounding box, same as
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
    // than deferring to Update() via a polled-and-cleared flag (same as CLoginWin::RmlClickOk()):
    // UpdateWhileActive() (this window's own Update() override now) can go many frames without
    // running at all under the legacy activation system this window has since migrated off of, so
    // a polled flag could sit unconsumed indefinitely. This listener firing IS proof the click
    // happened, with no need to wait for anything else to agree.
    void RmlClickMenu() { OpenSysMenu(); }
    void RmlClickCredit() { OpenCredits(); }

    // mu::ui::window::IObject
    bool Render() override
    {
        return true;
    }
    bool Update() override
    {
        return true;
    }
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
    void OpenSysMenu();
    void OpenCredits();

    Rml::ElementDocument* m_pRmlDoc = nullptr;
};

// Replaces CUIMng's old `CLoginMainWin m_LoginMainWin;` member, same convention as g_CreditWin.
extern CLoginMainWin g_LoginMainWin;

#endif // !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
