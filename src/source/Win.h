#pragma once

#include "UIBaseDef.h"
#include "PList.h"
#include "Sprite.h"

#define	WS_NORMAL				0
#define	WS_ETC					1
#define	WS_MOVE					2
#define	WS_EXTEND_DN			3
#define	WS_EXTEND_UP			4

#define	WA_ALL					0
#define	WA_TOOLTIP				1
#define	WA_MOVE					2
#define	WA_BUTTON				3
#define	WA_EXTEND_DN			4
#define	WA_EXTEND_UP			5

class CSprite;
class CButton;

class CWin
{
protected:
    POINT		m_ptHeld;

    bool		m_bShow;
    bool		m_bActive;
    bool		m_bDocking;
    int			m_nState;

    CSprite* m_psprBg;
    POINT		m_ptPos;
    SIZE		m_Size;
    POINT		m_ptTemp;

    CPList		m_BtnList;

public:
    CWin();
    virtual ~CWin();

    void Create(int nWidth, int nHeight, int nTexID = -1, bool bTile = false);
    virtual void Release();
    virtual void SetPosition(int nXCoord, int nYCoord);
    int GetXPos() { return m_ptPos.x; }
    int GetYPos() { return m_ptPos.y; }
    void SetSize(int nWidth, int nHeight, CHANGE_PRAM eChangedPram = XY);
    int GetWidth() { return m_Size.cx; }
    int GetHeight() { return m_Size.cy; }
    int GetTempXPos() { return m_ptTemp.x; }
    int GetTempYPos() { return m_ptTemp.y; }
    virtual bool CursorInWin(int nArea);
    virtual void Show(bool bShow);
    bool IsShow() { return m_bShow; }
    virtual void Active(bool bActive) { m_bActive = bActive; }
    bool IsActive() { return m_bActive; }
    void SetDocking(bool bDocking) { m_bDocking = bDocking; }
    int GetState() { return m_nState; }
    void Update(double dDeltaTick);
    virtual void Render();

    virtual int SetLine(int nLine) { return 0; }
    void ActiveBtns(bool bActive);

    BYTE GetBgAlpha() { return m_psprBg->GetAlpha(); }
    void SetBgAlpha(BYTE byAlpha) { m_psprBg->SetAlpha(byAlpha); }
    void SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue)
    {
        m_psprBg->SetColor(byRed, byGreen, byBlue);
    }

protected:
    virtual void PreRelease() {};
    virtual bool CursorInButtonlike() { return false; }
    virtual void UpdateWhileShow(double dDeltaTick) {};
    virtual void CheckAdditionalState() {};
    virtual void UpdateWhileActive(double dDeltaTick) {};
    virtual void RenderControls() {};

    void RegisterButton(CButton* pBtn);
    void RenderButtons();
};
