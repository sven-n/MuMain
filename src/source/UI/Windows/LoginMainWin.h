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

    // Invoked from the RmlUi document's click listeners (see Create()). Polled-and-cleared
    // exactly like the legacy CButton::IsClick() edge triggers they supplement in
    // UpdateWhileActive() -- mirrors CLoginWin::RmlClickOk()'s shape.
    void RmlClickMenu() { m_bRmlMenuClicked = true; }
    void RmlClickCredit() { m_bRmlCreditClicked = true; }

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);

private:
    bool m_bRmlMenuClicked = false;
    bool m_bRmlCreditClicked = false;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
};

#endif // !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
