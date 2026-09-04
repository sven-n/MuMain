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

#include "UI/NewUI/NewUIBase.h"
#include "UI/Widgets/Button.h"
#include "Render/Sprites/Sprite.h"

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

// CUIMng/CNewUIManager merger (docs/rmlui-ui-system) Phase 1 pilot: the first CUIMng window
// migrated off CWin onto SEASON3B::CNewUIObj/CSceneUICoordinator::GetNewStyleMng() -- the lowest-complexity
// real case (still fully legacy-2D, no RmlUi entanglement, no shown-vs-active split needed),
// chosen to prove the registry/dispatch mechanics before touching anything RmlUi-coupled. See
// g_CreditWin's own comment below for the ownership/registration shape.
class CCreditWin : public SEASON3B::CNewUIObj
{
	enum SHOW_STATE { HIDE, FADEIN, SHOW, FADEOUT };

	using DurationMs = std::chrono::duration<double, std::milli>;

	struct SCreditItem
	{
		std::uint8_t    byClass;
		char            szName[CRW_NAME_MAX];
	};

protected:
	// Replaces CWin::m_psprBg -- CWin::Create(w, h) (default nTexID=-1) created this as a full-
	// screen solid quad; CCreditWin::Create() immediately raised its alpha to fully opaque
	// (CWin::SetBgAlpha(255)) instead of the default 128 (semi-transparent).
	CSprite		m_sprBg;
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
	~CCreditWin() override;

	void Create();
	void Release(); // was CWin::PreRelease() (an override hook CWin::Release() called
	                 // automatically) -- called explicitly now, same call sites Create() itself
	                 // uses to reset state, plus wherever CSceneUICoordinator tears the login
	                 // scene down.
	void SetPosition();
	void Show(bool bShow) override;

	// SEASON3B::INewUIBase
	bool Render() override;
	bool Update() override;
	// Full-screen exclusive overlay: consumes every click while shown, matching the old
	// CWin::CursorInWin(WA_ALL)-while-shown behavior (a full-screen rect always hit-tests true).
	bool UpdateMouseEvent() override { return false; }
	// No focus-based key routing needed -- ESC is polled directly in Update(), same as before.
	bool UpdateKeyEvent() override { return true; }
	// Intentionally above every known CNewUIObj depth (5.5-11.0 range) -- full-screen exclusive
	// overlay, always on top of anything else registered with the same manager while shown.
	float GetLayerDepth() override { return 100.0f; }

protected:
	void CloseWin();
	void Init();
	void LoadIllust();
	void AnimationIllust(DurationMs deltaTime);
	void LoadText();
	void SetTextIndex();
	void AnimationText(int nClass, DurationMs deltaTime);
};

// Replaces CUIMng's old `CCreditWin m_CreditWin;` member -- static storage duration matches the
// same lifetime that singleton member had. Registers itself with
// CSceneUICoordinator::Instance().GetNewStyleMng() inside Create() (see that method), not with the shared
// g_pNewUIMng that MAIN_SCENE's windows use (CCreditWin only ever exists during LOG_IN_SCENE).
// External access (Winmain.cpp's RenderTextOnTop-style late pass, WSclient.cpp/LoginScene.cpp's
// IsVisible() checks, LoginMainWin.cpp's OpenCredits) goes through this global directly, same
// convention g_pSkillList already uses for the CNewUIObj tier.
extern CCreditWin g_CreditWin;

#endif // !defined(AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_)
