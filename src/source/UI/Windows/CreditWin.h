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

#include "UI/Core/WindowObject.h"
#include "Render/Sprites/Sprite.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

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

// The first CUIMng window migrated off CWin onto mu::ui::window::CObject/
// CSceneUICoordinator::GetNewStyleMng() -- the lowest-complexity real case (no shown-vs-active
// split needed). Stage 1 of its RmlUi port: the background/deco/logo and close button now live in
// #panel (credit_win.rml/.rcss). The illustration crossfade (m_aSpr[CRW_SPR_PIC_L/R]) and
// scrolling credit text (still drawn directly in Render() via g_pRenderText) remain native for now
// -- porting those needs a continuously-animated bound opacity value (no existing window does
// that yet) and confirming what character set credit.bmd's names need before picking an RmlUi font
// face, so they're deliberately left for a follow-up pass. See g_CreditWin's own comment below for
// the ownership/registration shape.
class CCreditWin : public mu::ui::window::CObject
{
	enum SHOW_STATE { HIDE, FADEIN, SHOW, FADEOUT };

	using DurationMs = std::chrono::duration<double, std::milli>;

	struct SCreditItem
	{
		std::uint8_t    byClass;
		char            szName[CRW_NAME_MAX];
	};

protected:
	// m_aSpr[CRW_SPR_DECO]/[CRW_SPR_LOGO] are unused now (Stage 1 moved them to #panel) but the
	// array keeps its original indices -- only [CRW_SPR_PIC_L]/[CRW_SPR_PIC_R] (illustration) and
	// [CRW_SPR_TXT_HIDE0..2] (credit-text fade overlays) are still Create()'d/rendered.
	CSprite		m_aSpr[CRW_SPR_MAX];

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

	// Invoked from the RmlUi document's data-event-click binding (see Create()). Polled-and-
	// cleared exactly like every other migrated window's RmlClickX() pattern.
	void RmlClickClose() { m_bRmlCloseClicked = true; }

	// mu::ui::window::IObject
	bool Render() override;
	bool Update() override;
	// Full-screen exclusive overlay: consumes every click while shown, matching the old
	// CWin::CursorInWin(WA_ALL)-while-shown behavior (a full-screen rect always hit-tests true).
	bool UpdateMouseEvent() override { return false; }
	// No focus-based key routing needed -- ESC is polled directly in Update(), same as before.
	bool UpdateKeyEvent() override { return true; }
	// Intentionally above every known CObject depth (5.5-11.0 range) -- full-screen exclusive
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

private:
	// Stage 1 has no dynamic content yet -- this model exists purely to host the close button's
	// data-event-click binding. Stage 2 (illustration crossfade / credit text) adds real fields.
	struct CreditWinRmlModel {};
	RmlModelBinder<CreditWinRmlModel> m_RmlBinder;
	Rml::ElementDocument* m_pRmlDoc = nullptr;

	bool m_bRmlCloseClicked = false;
};

// Replaces CUIMng's old `CCreditWin m_CreditWin;` member -- static storage duration matches the
// same lifetime that singleton member had. Registers itself with
// CSceneUICoordinator::Instance().GetNewStyleMng() inside Create() (see that method), not with the shared
// g_pNewUIMng that MAIN_SCENE's windows use (CCreditWin only ever exists during LOG_IN_SCENE).
// External access (Winmain.cpp's RenderTextOnTop-style late pass, WSclient.cpp/LoginScene.cpp's
// IsVisible() checks, LoginMainWin.cpp's OpenCredits) goes through this global directly, same
// convention g_pSkillList already uses for the CObject tier.
extern CCreditWin g_CreditWin;

#endif // !defined(AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_)
