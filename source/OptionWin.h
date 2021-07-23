//*****************************************************************************
// File: OptionWin.h
//
// Desc: interface for the COptionWin class.
//		 옵션 창 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_OPTIONWIN_H__D197E99D_D0C5_4E07_8095_68F233E7A428__INCLUDED_)
#define AFX_OPTIONWIN_H__D197E99D_D0C5_4E07_8095_68F233E7A428__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WinEx.h"
#include "Button.h"
#include "Slider.h"

#define	OW_BTN_AUTO_ATTACK		0	// 자동공격.
#define	OW_BTN_WHISPER_ALARM	1	// 귓말 알림음.
#define	OW_BTN_SLIDE_HELP		2	// 슬라이드 도움말.
#define	OW_BTN_CLOSE			3	// 닫기.
#define	OW_BTN_MAX				4	// 버튼 개수.

#define OW_SLD_EFFECT_VOL		0	// 효과음 조절.
#define OW_SLD_RENDER_LV		1	// +효과 제한.
#define OW_SLD_MAX				2	// 슬라이더 개수.

class COptionWin : public CWin  
{
protected:
	CWinEx		m_winBack;				// 배경.(배경 이미지로만 사용)
	CButton		m_aBtn[OW_BTN_MAX];		// 버튼들.
	CSlider		m_aSlider[OW_SLD_MAX];	// 슬라이더 컨트롤들.

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

#endif // !defined(AFX_OPTIONWIN_H__D197E99D_D0C5_4E07_8095_68F233E7A428__INCLUDED_)
