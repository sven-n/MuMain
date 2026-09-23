//*****************************************************************************
// File: LoginMainWin.h
//*****************************************************************************

#if !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
#define AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"

namespace Rml { class ElementDocument; }

// The login menu/credit button bar. Its two buttons are pure image buttons with no I18N text or
// dynamic state, so click detection is wired directly via Rml::Element::AddEventListener rather
// than an RmlModelBinder. RmlUi renders all of this bar's visuals. Not modal -- UpdateMouseEvent()
// claims only within its own bounding rect.
class CLoginMainWin : public mu::ui::window::CObject
{
protected:
    // This window's own bounding box (no shared rect facility on the CObject side).
    POINT m_ptPos;
    SIZE m_Size;

public:
    CLoginMainWin();
    ~CLoginMainWin() override;

    void Create();
    void Release();
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

    // Bound to the RmlUi document's click listeners; acts immediately rather than deferring to a
    // polled flag, since the listener firing is itself proof the click happened.
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
    // Claims any click within its own bounding box. Not modal.
    bool UpdateMouseEvent() override;
    bool UpdateKeyEvent() override
    {
        return true;
    }
    float GetLayerDepth() override
    {
        return 15.0f;
    }
    void ReloadRmlTheme();

private:
    void OpenSysMenu();
    void OpenCredits();
    void BuildRmlUi();

    Rml::ElementDocument* m_pRmlDoc = nullptr;
};

extern CLoginMainWin g_LoginMainWin;

#endif // !defined(AFX_LOGINMAINWIN_H__96B05A69_6360_4C8E_BD9C_20FC72EBE1C6__INCLUDED_)
