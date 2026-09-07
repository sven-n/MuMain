//*****************************************************************************
// File: CreditWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/CreditWin.h"
#include "Core/Input/Input.h"
#include "Core/Globals/_enum.h"
#include "UI/Core/SceneUICoordinator.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "App/Platform/Windows/Local.h"
#include "Core/Platform/PathResolve.h"

#include "UI/Widgets/UIControls.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cwchar>
#include <memory>
#include <string_view>

namespace
{
    using DurationMs = std::chrono::duration<double, std::milli>;

    constexpr DurationMs kIllustFadeDuration{2000.0};
    constexpr DurationMs kIllustShowDuration{22000.0};
    constexpr DurationMs kTextFadeDuration{1000.0};
    constexpr DurationMs kNameShowDuration{2300.0};
    constexpr std::string_view kCreditDataPath = "Data\\Local\\credit.bmd";

    constexpr std::array<std::array<const wchar_t*, 2>, CRW_ILLUST_MAX> kIllustPaths = {{
        {L"Interface\\im1_1.jpg", L"Interface\\im1_2.jpg"},
        {L"Interface\\im2_1.jpg", L"Interface\\im2_2.jpg"},
        {L"Interface\\im3_1.jpg", L"Interface\\im3_2.jpg"},
        {L"Interface\\im4_1.jpg", L"Interface\\im4_2.jpg"},
        {L"Interface\\im5_1.jpg", L"Interface\\im5_2.jpg"},
        {L"Interface\\im6_1.jpg", L"Interface\\im6_2.jpg"},
        {L"Interface\\im7_1.jpg", L"Interface\\im7_2.jpg"},
        {L"Interface\\im8_1.jpg", L"Interface\\im8_2.jpg"},
    }};

    template<typename T>
    short IncreaseAlpha(short alpha, T ratio)
    {
        const double delta = 255.0 * std::clamp(static_cast<double>(ratio), 0.0, 1.0);
        return static_cast<short>(std::min<double>(255.0, static_cast<double>(alpha) + delta));
    }

    template<typename T>
    short DecreaseAlpha(short alpha, T ratio)
    {
        const double delta = 255.0 * std::clamp(static_cast<double>(ratio), 0.0, 1.0);
        return static_cast<short>(std::max<double>(0.0, static_cast<double>(alpha) - delta));
    }

    template<std::size_t N>
    void CopyNameToWide(const char* source, wchar_t (&destination)[N])
    {
        if (source == nullptr)
        {
            destination[0] = L'\0';
            return;
        }

        std::mbstowcs(destination, source, N);
        destination[N - 1] = L'\0';
    }
}




// See this global's own header comment (CreditWin.h).
CCreditWin g_CreditWin;

CCreditWin::CCreditWin()
    : m_eIllustState(HIDE)
    , m_illustElapsed(DurationMs::zero())
    , m_byIllust(0)
    , m_illustPaths(kIllustPaths)
    , m_nNowIndex(0)
    , m_nNameCount(0)
    , m_anTextIndex{}
    , m_aeTextState{}
    , m_textElapsed(DurationMs::zero())
{
	// Not SetLayoutMode() here -- CManager::AddUIObj() (called from Create(), below)
	// overwrites it unconditionally on first registration via UI::Layout::ForInterface(), which
	// is the actual authority; see that policy table's own INTERFACE_CREDITS entry
	// (UILayoutPolicy.cpp) for why this window needs LayoutMode::Legacy specifically.
}

CCreditWin::~CCreditWin()
{

}

void CCreditWin::Create()
{
	// Mirrors CWin::Create()'s own internal Release()-then-rebuild pattern -- avoids leaking the
	// sprites on a second Create() call (RepositionSceneUI()'s resolution-change path).
	Release();

	LoadText();

	// RmlUi migration -- see this class's header comment. Guarded like every other hybrid window's
	// Create() (CSceneUICoordinator::RepositionSceneUI() re-runs Create() on resolution change), so
	// the document/model are created once, ever.
	if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
	{
		const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "credit_win",
			[this](Rml::DataModelConstructor& c, CreditWinRmlModel& model)
			{
				c.BindEventCallback("creditwin_close_click",
					[this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickClose(); });

				c.Bind("department", &model.department);
				c.Bind("team", &model.team);
				c.Bind("department_opacity", &model.departmentOpacity);
				c.Bind("team_opacity", &model.teamOpacity);
				c.Bind("names_opacity", &model.namesOpacity);

				// One credit-name slot -- same RegisterStruct/RegisterArray shape as CBuffStrip's
				// own data-for list (BuffStrip.cpp).
				auto name = c.RegisterStruct<CreditNameEntry>();
				name.RegisterMember("text", &CreditNameEntry::text);
				c.RegisterArray<std::vector<CreditNameEntry>>();
				c.Bind("names", &model.names);

				c.Bind("illust_left_decorator", &model.illustLeftDecorator);
				c.Bind("illust_right_decorator", &model.illustRightDecorator);
				c.Bind("illust_opacity", &model.illustOpacity);
			});

		if (modelCreated)
			m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/credit_win.rml");
	}

	// Registers with CSceneUICoordinator's
	// own scene-scoped manager instance, not the shared g_pNewUIMng (this window only ever exists
	// during LOG_IN_SCENE; see CSceneUICoordinator::GetNewStyleMng()'s own comment for why).
	// AddUIObj() is already idempotent (no-ops if already registered), so this is safe to call
	// again on every RepositionSceneUI()-triggered recreate.
	CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_CREDITS, this);

	// Matches CWin::Create()'s own unconditional m_bShow=false reset -- RepositionSceneUI()
	// snapshots visibility before calling this and restores it right after.
	Show(false);
}

void CCreditWin::Release()
{
	// See CLoginWin::PreRelease()'s identical comment -- each migrated window's Release() is
	// called explicitly at every scene transition, not swept automatically by any shared list, and
	// this class has no base-class knowledge of m_pRmlDoc.
	if (m_pRmlDoc)
		m_pRmlDoc->Hide();
}

void CCreditWin::Show(bool bShow)
{
	mu::ui::window::CObject::Show(bShow);

	if (m_pRmlDoc)
	{
		if (bShow) m_pRmlDoc->Show();
		else       m_pRmlDoc->Hide();
	}

	if (bShow)
		Init();
	else
		m_eIllustState = HIDE;
}

bool CCreditWin::Update()
{
	// g_pTimer is never reset, so GetTimeElapsed() is total process uptime, not a per-frame delta.
	// SceneManager.cpp's own dDeltaTick (what CWin::Update() used to pass this window as its
	// deltaMilliseconds parameter) only behaves like a delta because it's clamped to
	// 200.0 * FPS_ANIMATION_FACTOR, a ceiling the raw uptime value exceeds almost immediately and
	// forever after -- in steady state it reduces to exactly that clamped constant, so this reads
	// the same effective value directly instead of going through g_pTimer at all.
	extern float FPS_ANIMATION_FACTOR;
	const DurationMs deltaTime{ 200.0 * static_cast<double>(FPS_ANIMATION_FACTOR) };

	if (m_bRmlCloseClicked)
	{
		m_bRmlCloseClicked = false;
		CloseWin();
	}
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		::PlayBuffer(SOUND_CLICK01);
		CloseWin();
	}

	for (int i = 0; i <= CRW_INDEX_NAME; ++i)
		AnimationText(i, deltaTime);
	AnimationIllust(deltaTime);

	SyncRmlModel();

	return true;
}

bool CCreditWin::Render()
{
	// RmlUi's #panel now owns 100% of this window's visuals -- see this class's header comment.
	// Nothing left to draw here; SyncRmlModel() (called from Update()) is what keeps the model
	// current.
	return true;
}

void CCreditWin::CloseWin()
{
	Show(false);

	SocketClient->ToConnectServer()->SendServerListRequest();

	::StopMp3(MUSIC_MUTHEME);
	::PlayMp3(MUSIC_MAIN_THEME);
}

void CCreditWin::Init()
{
	m_eIllustState = FADEIN;
	m_illustElapsed = DurationMs::zero();
	m_byIllust = 0;
	m_nIllustAlpha = 0;

	for (int i = 0; i <= CRW_INDEX_NAME; ++i)
		m_aeTextState[i] = FADEIN;
	m_textElapsed = DurationMs::zero();
	m_nNowIndex = 0;
	m_nNameCount = 0;
	SetTextIndex();
}

// Was CSprite+LoadBitmap/BITMAP_TEMP before Stage 2 -- the two illustration <img>s
// (credit_win.rml) now load m_illustPaths[m_byIllust][0/1] directly from disk via a C++-swapped
// data-attr-src, same technique loading.rml's own background tiles already use for static files;
// SyncRmlModel() pushes the path whenever m_byIllust changes and m_nIllustAlpha (below) every frame
// while fading. The state machine/timing themselves (kIllustFadeDuration/kIllustShowDuration) are
// unchanged from before this port.
void CCreditWin::AnimationIllust(DurationMs deltaTime)
{
	switch (m_eIllustState)
	{
	case FADEIN:
		m_nIllustAlpha = IncreaseAlpha(m_nIllustAlpha, deltaTime / kIllustFadeDuration);
		if (255 <= m_nIllustAlpha)
		{
			m_eIllustState = SHOW;
			m_nIllustAlpha = 255;
		}
		break;

	case SHOW:
		m_illustElapsed += deltaTime;
		if (m_illustElapsed > kIllustShowDuration)
		{
			m_eIllustState = FADEOUT;
			m_illustElapsed = DurationMs::zero();
		}
		break;

	case FADEOUT:
		m_nIllustAlpha = DecreaseAlpha(m_nIllustAlpha, deltaTime / kIllustFadeDuration);
		if (0 >= m_nIllustAlpha)
		{
			m_eIllustState = FADEIN;
			m_nIllustAlpha = 0;

			m_byIllust = ++m_byIllust == CRW_ILLUST_MAX ? 0 : m_byIllust;
		}
		break;
	}
}

void CCreditWin::LoadText()
{
#ifdef _WIN32
	std::unique_ptr<FILE, decltype(&std::fclose)> file(std::fopen(kCreditDataPath.data(), "rb"), &std::fclose);
#else
	// The path is Windows-spelled (backslashes, mixed case); resolve it against
	// the case-sensitive filesystem.
	std::unique_ptr<FILE, decltype(&std::fclose)> file(std::fopen(MuResolvePath(kCreditDataPath.data()).c_str(), "rb"), &std::fclose);
#endif
	if (!file)
	{
		wchar_t szMessage[256];
		std::swprintf(szMessage, std::size(szMessage), L"%hs file not found.\r\n", kCreditDataPath.data());
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}

	const std::size_t nSize = sizeof(SCreditItem) * CRW_ITEM_MAX;
	if (std::fread(m_aCredit, nSize, 1, file.get()) != 1)
	{
		wchar_t szMessage[256];
		std::swprintf(szMessage, std::size(szMessage), L"Failed to read %hs file or file is corrupt.\r\n", kCreditDataPath.data());
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}
	::BuxConvert(reinterpret_cast<BYTE*>(m_aCredit), static_cast<int>(nSize));
}

void CCreditWin::SetTextIndex()
{
	if (0 == m_aCredit[m_nNowIndex].byClass)
	{
		::PlayBuffer(SOUND_CLICK01);
		CloseWin();
	}

	if (1 == m_aCredit[m_nNowIndex].byClass)
	{
		m_anTextIndex[CRW_INDEX_DEPARTMENT] = m_nNowIndex;
		++m_nNowIndex;
	}
	if (2 == m_aCredit[m_nNowIndex].byClass)
	{
		m_anTextIndex[CRW_INDEX_TEAM] = m_nNowIndex;
		++m_nNowIndex;
	}

	int iNameCnt = 0;
	for (int i = 0; i < 4; ++i)
	{
		iNameCnt = i;
		if (3 == m_aCredit[m_nNowIndex].byClass)
		{
			m_anTextIndex[CRW_INDEX_NAME0 + i] = m_nNowIndex;
			++m_nNowIndex;
		}
		else
			break;
	}
	m_nNameCount = iNameCnt;
}

void CCreditWin::AnimationText(int nClass, DurationMs deltaTime)
{
	SHOW_STATE* peTextState = &m_aeTextState[nClass];
	short& nAlpha = m_anTextAlpha[nClass];

	switch (*peTextState)
	{
	case FADEIN:
		// Text-visible alpha rising 0->255 -- was a black hide-overlay sprite's alpha falling
		// 255->0 before Stage 2; see m_anTextAlpha's own header comment.
		nAlpha = IncreaseAlpha(nAlpha, deltaTime / kTextFadeDuration);
		if (255 <= nAlpha)
		{
			*peTextState = SHOW;
			nAlpha = 255;
		}
		break;

	case SHOW:
		if (nClass != CRW_INDEX_NAME)
			break;

		m_textElapsed += deltaTime;
		if (m_textElapsed > kNameShowDuration)
		{
			m_aeTextState[CRW_INDEX_NAME] = FADEOUT;
			m_textElapsed = DurationMs::zero();

			if (3 != m_aCredit[m_nNowIndex].byClass)
			{
				m_aeTextState[CRW_INDEX_TEAM] = FADEOUT;
				if (2 != m_aCredit[m_nNowIndex].byClass)
					m_aeTextState[CRW_INDEX_DEPARTMENT] = FADEOUT;
			}
		}
		break;

	case FADEOUT:
		nAlpha = DecreaseAlpha(nAlpha, deltaTime / kTextFadeDuration);
		if (0 >= nAlpha)
		{
			*peTextState = FADEIN;
			nAlpha = 0;

			if (nClass == CRW_INDEX_NAME)
				SetTextIndex();
		}
		break;
	}
}

void CCreditWin::SyncRmlModel()
{
	if (!m_pRmlDoc) return;

	auto syncText = [this](Rml::String CreditWinRmlModel::* field, const char* boundName, const wchar_t* text)
	{
		const std::string utf8 = StringUtils::WideToNarrow(text);
		if (m_RmlBinder.GetModel().*field != utf8)
		{
			m_RmlBinder.GetModel().*field = utf8;
			m_RmlBinder.MarkDirty(boundName);
		}
	};
	auto syncFloat = [this](float CreditWinRmlModel::* field, const char* boundName, float value)
	{
		if (m_RmlBinder.GetModel().*field != value)
		{
			m_RmlBinder.GetModel().*field = value;
			m_RmlBinder.MarkDirty(boundName);
		}
	};
	// Named-sprite decorator string, e.g. "image(illust-im3-1)" -- matches one of the 16
	// single-rect @spritesheet blocks in credit_win.rcss (illust-im1-1 .. illust-im8-2, one per
	// im{N}_{M}.jpg). Deliberately NOT a raw <img data-attr-src> (this class's first attempt):
	// that route needs its own path convention through two independent, easy-to-get-wrong layers
	// (RenderManager::LoadTexture()'s JoinPath() call, then RmlUiRenderInterface::LoadTexture()'s
	// own routing through CGlobalBitmap -- see git history on this file for what that took to get
	// loading at all) and even once the file loads correctly, the rendered image doesn't fill its
	// box -- a real, uninvestigated gap between the two illustrations, root cause not found. The
	// decorator/@spritesheet route reuses the exact mechanism this document's own logo/deco/close-
	// button decorators already prove works, and CBuffStrip's per-instance data-style-decorator
	// (BuffStrip.cpp/buff_strip.rml) already proves a decorator string can be swapped from C++ at
	// runtime the same way.
	auto illustDecorator = [](std::uint8_t illustIndex, int side)
	{
		return "image(illust-im" + std::to_string(illustIndex + 1) + "-" + std::to_string(side + 1) + ")";
	};

	wchar_t buffer[CRW_NAME_MAX]{};
	CopyNameToWide(m_aCredit[m_anTextIndex[CRW_INDEX_DEPARTMENT]].szName, buffer);
	syncText(&CreditWinRmlModel::department, "department", buffer);
	CopyNameToWide(m_aCredit[m_anTextIndex[CRW_INDEX_TEAM]].szName, buffer);
	syncText(&CreditWinRmlModel::team, "team", buffer);

	syncFloat(&CreditWinRmlModel::departmentOpacity, "department_opacity", m_anTextAlpha[CRW_INDEX_DEPARTMENT] / 255.0f);
	syncFloat(&CreditWinRmlModel::teamOpacity, "team_opacity", m_anTextAlpha[CRW_INDEX_TEAM] / 255.0f);
	syncFloat(&CreditWinRmlModel::namesOpacity, "names_opacity", m_anTextAlpha[CRW_INDEX_NAME] / 255.0f);

	// Small (<=4-entry) list, rebuilt+marked-dirty unconditionally every frame -- same reasoning
	// CBuffStrip's own SyncRmlModel() documents for its data-for'd buff list.
	auto& model = m_RmlBinder.GetModel();
	model.names.clear();
	for (int i = 0; i < m_nNameCount; ++i)
	{
		CopyNameToWide(m_aCredit[m_anTextIndex[CRW_INDEX_NAME0 + i]].szName, buffer);
		model.names.push_back({ StringUtils::WideToNarrow(buffer) });
	}
	m_RmlBinder.MarkDirty("names");

	const std::string illustLeftDecorator = illustDecorator(m_byIllust, 0);
	if (model.illustLeftDecorator != illustLeftDecorator)
	{
		model.illustLeftDecorator = illustLeftDecorator;
		m_RmlBinder.MarkDirty("illust_left_decorator");
	}
	const std::string illustRightDecorator = illustDecorator(m_byIllust, 1);
	if (model.illustRightDecorator != illustRightDecorator)
	{
		model.illustRightDecorator = illustRightDecorator;
		m_RmlBinder.MarkDirty("illust_right_decorator");
	}
	syncFloat(&CreditWinRmlModel::illustOpacity, "illust_opacity", m_nIllustAlpha / 255.0f);
}
