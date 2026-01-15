//*****************************************************************************
// File: ServerSelWin.h
//*****************************************************************************
#pragma once

#include "Win.h"
#include "Button.h"
#include "GaugeBar.h"
#include "WinEx.h"

#define SSW_SERVER_G_MAX	21
#define SSW_SERVER_MAX		16
#define SSW_DESC_LINE_MAX	2
#define SSW_DESC_ROW_MAX	83
#define SSW_LEFT_SERVER_G_MAX	10
#define SSW_RIGHT_SERVER_G_MAX	10

class CServerGroup;

class CServerSelWin : public CWin
{
private:
    enum SERVER_SELECT_WIN
    {
        SERVER_GROUP_BTN_WIDTH = 108,
        SERVER_GROUP_BTN_HEIGHT = 26,
        SERVER_BTN_WIDTH = 193,
        SERVER_BTN_HEIGHT = 26,
    };

protected:
    CButton		m_aServerGroupBtn[SSW_SERVER_G_MAX];
    CButton		m_aServerBtn[SSW_SERVER_MAX];
    CGaugeBar	m_aServerGauge[SSW_SERVER_MAX];
    CSprite		m_aBtnDeco[2];
    CSprite		m_aArrowDeco[2];
    CWinEx		m_winDescription;

    int			m_icntServerGroup;
    int			m_icntLeftServerGroup;
    int			m_icntRightServerGroup;
    int			m_icntServer;
    bool		m_bTestServerBtn;

    int				m_iSelectServerBtnIndex;
    CServerGroup* m_pSelectServerGroup;

    wchar_t		m_szDescription[SSW_DESC_LINE_MAX][SSW_DESC_ROW_MAX];

public:
    CServerSelWin();
    virtual ~CServerSelWin();
    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void UpdateDisplay();
    void Show(bool bShow);
    bool CursorInWin(int nArea);

protected:
    void PreRelease();
    void SetServerBtnPosition();
    void SetArrowSpritePosition();
    void ShowServerGBtns();
    void ShowDecoSprite();
    void ShowArrowSprite();
    void ShowServerBtns();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();
};
