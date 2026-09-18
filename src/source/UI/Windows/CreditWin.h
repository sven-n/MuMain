//*****************************************************************************
// File: CreditWin.h
//*****************************************************************************

#if !defined(AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_)
#define AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_

#pragma once

#include <chrono>
#include <cstdint>
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

// Full-screen credits overlay, fully rendered via RmlUi (#panel, credit_win.rml/.rcss): scrolling
// department/team/name text and a two-illustration crossfade, both driven by this class's model.
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
	// Illustration-visible alpha (0=hidden, 255=fully shown). float, not short/BYTE -- an integer
	// accumulator stalls under uncapped FPS (see IncreaseAlpha()/DecreaseAlpha() in CreditWin.cpp).
	float		m_nIllustAlpha{};

	SCreditItem	m_aCredit[CRW_ITEM_MAX];
	int			m_nNowIndex;
	int			m_nNameCount;
	int			m_anTextIndex[CRW_INDEX_MAX];
	SHOW_STATE	m_aeTextState[CRW_INDEX_NAME + 1];
	// Text-visible alpha (0=hidden, 255=fully shown) for the DEPARTMENT/TEAM/NAME classes.
	float		m_anTextAlpha[CRW_INDEX_NAME + 1]{};
	DurationMs	m_textElapsed;

public:
	CCreditWin();
	~CCreditWin() override;

	void Create();
	void Release();
	void Show(bool bShow) override;

	// Set by the RmlUi close button's click binding; polled and cleared in Update().
	void RmlClickClose() { m_bRmlCloseClicked = true; }

	// mu::ui::window::IObject
	bool Render() override;
	bool Update() override;
	// Full-screen exclusive overlay: consumes every click while shown.
	bool UpdateMouseEvent() override { return false; }
	// ESC is polled directly in Update(); no focus-based key routing needed.
	bool UpdateKeyEvent() override { return true; }
	// Above every other CObject depth: full-screen exclusive, always on top while shown.
	float GetLayerDepth() override { return 100.0f; }

protected:
	void CloseWin();
	void Init();
	void AnimationIllust(DurationMs deltaTime);
	void LoadText();
	void SetTextIndex();
	void AnimationText(int nClass, DurationMs deltaTime);

private:
	// One credit-name slot; names is a variable-length (1-4) data-for'd list.
	struct CreditNameEntry { Rml::String text; };

	struct CreditWinRmlModel
	{
		Rml::String department, team;
		// Mirror m_aeTextState/m_anTextAlpha's 3 classes (DEPARTMENT/TEAM/NAME); pushed every frame while fading.
		float departmentOpacity = 0.f, teamOpacity = 0.f, namesOpacity = 0.f;
		std::vector<CreditNameEntry> names;

		// Illustration crossfade: decorator string swapped whenever m_byIllust advances, opacity
		// pushed every frame while fading.
		Rml::String illustLeftDecorator, illustRightDecorator;
		float illustOpacity = 0.f;
	};
	RmlModelBinder<CreditWinRmlModel> m_RmlBinder;
	Rml::ElementDocument* m_pRmlDoc = nullptr;

	bool m_bRmlCloseClicked = false;

	void SyncRmlModel();
};

// Registers with CSceneUICoordinator's scene-scoped manager in Create(); only exists during LOG_IN_SCENE.
extern CCreditWin g_CreditWin;

#endif // !defined(AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_)
