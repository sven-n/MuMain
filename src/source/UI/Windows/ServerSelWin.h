//*****************************************************************************
// File: ServerSelWin.h
//*****************************************************************************
#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/GaugeBar.h"
#include "UI/Widgets/WinEx.h"
#include "Render/Sprites/Sprite.h"

#define SSW_SERVER_G_MAX 21
#define SSW_SERVER_MAX 16
#define SSW_DESC_LINE_MAX 2
#define SSW_DESC_ROW_MAX 83
#define SSW_LEFT_SERVER_G_MAX 10
#define SSW_RIGHT_SERVER_G_MAX 10

class CServerGroup;

// CUIMng/CNewUIManager merger (docs/rmlui-ui-system) Phase 2: migrated off CWin the same way as
// CCreditWin/CServerMsgWin. The most interactive of the three so far (real buttons + gauge bars,
// not just passive display), but still fully self-contained real-pixel 2D rendering, no RmlUi
// entanglement. Its own CWinEx member (m_winDescription) is untouched -- CWinEx itself isn't
// retiring, only CServerSelWin's own CWin base class is.
class CServerSelWin : public SEASON3B::CNewUIObj
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
    CButton m_aServerGroupBtn[SSW_SERVER_G_MAX];
    CButton m_aServerBtn[SSW_SERVER_MAX];
    CGaugeBar m_aServerGauge[SSW_SERVER_MAX];
    CSprite m_aBtnDeco[2];
    CSprite m_aArrowDeco[2];
    CWinEx m_winDescription;

    // Replaces CWin::m_ptPos/m_Size -- no shared rect facility on the CNewUIObj side (matching
    // every pre-existing CNewUIObj window), so this window keeps its own bounding box, same as
    // CWin::CursorInWin(WA_ALL) used for hit-testing (ported into UpdateMouseEvent() below).
    POINT m_ptPos;
    SIZE m_Size;

    int m_icntServerGroup;
    int m_icntLeftServerGroup;
    int m_icntRightServerGroup;
    int m_icntServer;
    bool m_bTestServerBtn;

    int m_iSelectServerBtnIndex;
    CServerGroup* m_pSelectServerGroup;

    wchar_t m_szDescription[SSW_DESC_LINE_MAX][SSW_DESC_ROW_MAX];

public:
    CServerSelWin();
    ~CServerSelWin() override;
    void Create();
    void Release(); // was CWin::PreRelease() (an override hook CWin::Release() called
                     // automatically) -- called explicitly now, same as CCreditWin's own Release().
    void SetPosition(int nXCoord, int nYCoord);
    void UpdateDisplay();
    void Show(bool bShow) override;
    int GetWidth() const
    {
        return m_Size.cx;
    }
    int GetHeight() const
    {
        return m_Size.cy;
    }

    // SEASON3B::INewUIBase
    bool Render() override;
    bool Update() override;
    // Was CWin::CursorInWin(WA_ALL) -- claims (consumes) any click within its own bounding box,
    // same rect CUIMng's old activation walk used to decide which m_WinList window a click
    // belonged to. Doesn't need CCreditWin's full-screen-exclusive semantics (this window's own
    // rect, not the whole screen), and coexists fine with CCreditWin's depth-100 override: while
    // CCreditWin is shown it always wins the sort-by-depth dispatch regardless of this window's
    // own depth.
    bool UpdateMouseEvent() override;
    bool UpdateKeyEvent() override
    {
        return true;
    }
    float GetLayerDepth() override
    {
        return 20.0f;
    }

protected:
    void SetServerBtnPosition();
    void SetArrowSpritePosition();
    void ShowServerGBtns();
    void ShowDecoSprite();
    void ShowArrowSprite();
    void ShowServerBtns();
};

// Replaces CUIMng's old `CServerSelWin m_ServerSelWin;` member, same convention as g_CreditWin.
extern CServerSelWin g_ServerSelWin;
