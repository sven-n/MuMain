//*****************************************************************************
// File: ServerMsgWin.h
//*****************************************************************************

#if !defined(AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_)
#define AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "Render/Sprites/Sprite.h"

#define SMW_MSG_LINE_MAX 5
#define SMW_MSG_ROW_MAX 83

// CUIMng/CNewUIManager merger (docs/rmlui-ui-system) Phase 2: the first of the remaining
// still-legacy-2D CUIMng windows migrated off CWin/CWinEx, following CCreditWin's pattern.
// Purely passive/non-interactive (its old CWinEx::CursorInWin(WA_ALL) override always returned
// false, so it could never become CUIMng's "active" window -- confirmed no drag/resize behavior
// was ever reachable in practice), so unlike CCreditWin it doesn't need to consume clicks at all.
class CServerMsgWin : public mu::ui::window::CObject
{
    // Was CWinEx's WE_BG_* (WinEx.h) -- kept private here instead of reusing those shared macros
    // since this window no longer goes through CWinEx (still used by other not-yet-migrated
    // CWin/CWinEx windows: SysMenuWin).
    enum { BG_CENTER, BG_TOP, BG_BOTTOM, BG_LEFT, BG_RIGHT, BG_MAX };

protected:
    // Replaces CWinEx's m_psprBg[WE_BG_MAX] composite background (5-part 9-slice-style border).
    CSprite m_aSprBg[BG_MAX];
    POINT m_ptPos;
    int m_nBgSideNow;

    wchar_t m_aszMsg[SMW_MSG_LINE_MAX][SMW_MSG_ROW_MAX];
    int m_nMsgLine;

public:
    CServerMsgWin();
    ~CServerMsgWin() override;

    void Create();
    void Release(); // was CWinEx::Release() (invoked automatically via ~CWin());
                     // called explicitly now, same as CCreditWin's own Release().
    void SetPosition(int nXCoord, int nYCoord);
    void AddMsg(wchar_t* pszMsg);
    void Show(bool bShow) override;

    // mu::ui::window::IObject
    bool Render() override;
    // Never consumes -- purely passive message log, never intercepted clicks even as a CWin.
    bool UpdateMouseEvent() override { return true; }
    bool UpdateKeyEvent() override { return true; }
    // Intentionally low: sits alongside HUD-ish overlays, well below CCreditWin's full-screen
    // exclusive 100.0f, but this window and CCreditWin never coexist (login vs. character scene).
    float GetLayerDepth() override { return 10.0f; }

protected:
    int SetLine(int nLine);
};

// Replaces CUIMng's old `CServerMsgWin m_ServerMsgWin;` member, same convention as g_CreditWin.
extern CServerMsgWin g_ServerMsgWin;

#endif // !defined(AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_)
