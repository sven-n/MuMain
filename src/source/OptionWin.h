//*****************************************************************************
// File: OptionWin.h
//*****************************************************************************
#pragma once

#include "WinEx.h"
#include "Button.h"
#include "Slider.h"

#define	OW_BTN_AUTO_ATTACK		0
#define	OW_BTN_WHISPER_ALARM	1
#define	OW_BTN_SLIDE_HELP		2
#define	OW_BTN_CLOSE			3
#define	OW_BTN_MAX				4

#define OW_SLD_EFFECT_VOL		0
#define OW_SLD_RENDER_LV		1
#define OW_SLD_MAX				2

class COptionWin : public CWin
{
protected:
    CWinEx		m_winBack;
    CButton		m_aBtn[OW_BTN_MAX];
    CSlider		m_aSlider[OW_SLD_MAX];

public:
    COptionWin();
    virtual ~COptionWin();

    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
    bool CursorInWin(int nArea);
    void UpdateDisplay();

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();
};
