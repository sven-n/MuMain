#pragma once

#include "WinEx.h"
#include "Button.h"

#define	SMW_BTN_GAME_END	0
#define	SMW_BTN_SERVER_SEL	1
#define	SMW_BTN_OPTION		2
#define	SMW_BTN_CLOSE		3
#define	SMW_BTN_MAX			4

class CSysMenuWin : public CWin
{
protected:
    CWinEx		m_winBack;
    CButton		m_aBtn[SMW_BTN_MAX];

public:
    CSysMenuWin();
    virtual ~CSysMenuWin();

    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
    bool CursorInWin(int nArea);

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();
};
