//*****************************************************************************
// File: CreditWin.h
//*****************************************************************************

#if !defined(AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_)
#define AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_

#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>

#include "Win.h"
#include "Button.h"

#define	CRW_SPR_PIC_L			0
#define	CRW_SPR_PIC_R			1
#define	CRW_SPR_DECO			2
#define	CRW_SPR_LOGO			3
#define CRW_SPR_TXT_HIDE0		4
#define CRW_SPR_TXT_HIDE1		5
#define CRW_SPR_TXT_HIDE2		6
#define	CRW_SPR_MAX				7
#define CRW_ILLUST_MAX			8

#define	CRW_NAME_MAX			32
#define	CRW_ITEM_MAX			400

#define	CRW_INDEX_DEPARTMENT	0
#define	CRW_INDEX_TEAM			1
#define	CRW_INDEX_NAME			2
#define	CRW_INDEX_NAME0			2
#define	CRW_INDEX_NAME1			3
#define	CRW_INDEX_NAME2			4
#define	CRW_INDEX_NAME3			5
#define	CRW_INDEX_MAX			6

class CCreditWin : public CWin
{
	enum SHOW_STATE { HIDE, FADEIN, SHOW, FADEOUT };

	using DurationMs = std::chrono::duration<double, std::milli>;

	struct SCreditItem
	{
		std::uint8_t    byClass;
		char            szName[CRW_NAME_MAX];
	};

protected:
	CSprite		m_aSpr[CRW_SPR_MAX];
	CButton		m_btnClose;

	SHOW_STATE  m_eIllustState;
	DurationMs  m_illustElapsed;
	std::uint8_t        m_byIllust;
	std::array<std::array<const wchar_t*, 2>, CRW_ILLUST_MAX> m_illustPaths;

	std::unique_ptr<std::remove_pointer_t<HFONT>, void(*)(HFONT)>	m_font;
	SCreditItem	m_aCredit[CRW_ITEM_MAX];
	int			m_nNowIndex;
	int			m_nNameCount;
	int			m_anTextIndex[CRW_INDEX_MAX];
	SHOW_STATE	m_aeTextState[CRW_INDEX_NAME + 1];
	DurationMs	m_textElapsed;

public:
	CCreditWin();
	virtual ~CCreditWin();

	void Create();
	void SetPosition();
	void Show(bool bShow);
	bool CursorInWin(int nArea);

protected:
	void PreRelease();
	void UpdateWhileActive(double dDeltaTick);
	void RenderControls();

	void CloseWin();
	void Init();
	void LoadIllust();
	void AnimationIllust(DurationMs deltaTime);
	void LoadText();
	void SetTextIndex();
	void AnimationText(int nClass, DurationMs deltaTime);
};

#endif // !defined(AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_)
