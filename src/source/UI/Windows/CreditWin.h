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
#include <vector>

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

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
// split needed). Fully on RmlUi now (#panel, credit_win.rml/.rcss): Stage 1 moved the background/
// deco/logo and close button; Stage 2 moved the scrolling credit text (department/team/names,
// opacity-faded via the model instead of g_pRenderText + a black hide-overlay sprite) and the
// illustration crossfade (two named-@spritesheet decorators, C++-swapped via a per-instance
// data-style-decorator the same way CBuffStrip's buff icons already are, plus a model-pushed
// opacity, instead of CSprite + LoadBitmap/BITMAP_TEMP -- see AnimationIllust()'s own comment).
// See g_CreditWin's own comment below for the ownership/registration shape.
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
	SHOW_STATE  m_eIllustState;
	DurationMs  m_illustElapsed;
	std::uint8_t        m_byIllust;
	std::array<std::array<const wchar_t*, 2>, CRW_ILLUST_MAX> m_illustPaths;
	// Illustration-visible alpha (0=hidden, 255=fully shown) -- was CSprite::GetAlpha()/SetAlpha()
	// on the two illustration sprites before Stage 2; tracked directly now since there's no sprite
	// to read it back from.
	short		m_nIllustAlpha{};

	SCreditItem	m_aCredit[CRW_ITEM_MAX];
	int			m_nNowIndex;
	int			m_nNameCount;
	int			m_anTextIndex[CRW_INDEX_MAX];
	SHOW_STATE	m_aeTextState[CRW_INDEX_NAME + 1];
	// Text-visible alpha (0=hidden, 255=fully shown) for the DEPARTMENT/TEAM/NAME classes -- was
	// the complementary value of a black hide-overlay sprite's own alpha before Stage 2; tracked
	// directly now since there's no overlay sprite to read it back from.
	short		m_anTextAlpha[CRW_INDEX_NAME + 1]{};
	DurationMs	m_textElapsed;

public:
	CCreditWin();
	~CCreditWin() override;

	void Create();
	void Release(); // was CWin::PreRelease() (an override hook CWin::Release() called
	                 // automatically) -- called explicitly now, same call sites Create() itself
	                 // uses to reset state, plus wherever CSceneUICoordinator tears the login
	                 // scene down.
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
	void AnimationIllust(DurationMs deltaTime);
	void LoadText();
	void SetTextIndex();
	void AnimationText(int nClass, DurationMs deltaTime);

private:
	// One credit-name slot -- names is a variable-length (1-4) data-for'd list, same shape as
	// CBuffStrip's own BuffEntry/data-for pattern (BuffStrip.cpp/buff_strip.rml).
	struct CreditNameEntry { Rml::String text; };

	struct CreditWinRmlModel
	{
		Rml::String department, team;
		// Mirror m_aeTextState/m_anTextAlpha's 3 classes (DEPARTMENT/TEAM/NAME) -- pushed every
		// frame while fading, same convention as MainFrameWindow's hp_fraction/mp_fraction etc.
		float departmentOpacity = 0.f, teamOpacity = 0.f, namesOpacity = 0.f;
		std::vector<CreditNameEntry> names;

		// Illustration crossfade -- decorator swapped whenever m_byIllust advances (a plain "image(
		// sprite-name)" string, same technique CBuffStrip's own per-instance data-style-decorator
		// uses, BuffStrip.cpp/buff_strip.rml -- proven working already, unlike a raw <img src>,
		// which is what this used originally; see credit_win.rcss's own comment for why it
		// changed), opacity pushed every frame while fading (AnimationIllust()'s own comment).
		Rml::String illustLeftDecorator, illustRightDecorator;
		float illustOpacity = 0.f;
	};
	RmlModelBinder<CreditWinRmlModel> m_RmlBinder;
	Rml::ElementDocument* m_pRmlDoc = nullptr;

	bool m_bRmlCloseClicked = false;

	void SyncRmlModel();
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
