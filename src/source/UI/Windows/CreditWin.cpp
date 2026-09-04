//*****************************************************************************
// File: CreditWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "Render/Renderer/MuRenderer.h"
#include "UI/Windows/CreditWin.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Core/Input/Input.h"
#include "Core/Globals/_enum.h"
#include "UI/Legacy/UIMng.h"
#include "UI/Scaling/UITransform.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "App/Platform/Windows/Local.h"
#include "I18N/All.h"
#include "Core/Platform/PathResolve.h"

#include "UI/Legacy/UIControls.h"

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

    void FontDeleter(HFONT font)
    {
        if (font != nullptr)
        {
            ::DeleteObject(font);
        }
    }
}




// See this global's own header comment (CreditWin.h).
CCreditWin g_CreditWin;

CCreditWin::CCreditWin()
    : m_eIllustState(HIDE)
    , m_illustElapsed(DurationMs::zero())
    , m_byIllust(0)
    , m_illustPaths(kIllustPaths)
    , m_font(nullptr, &FontDeleter)
    , m_nNowIndex(0)
    , m_nNameCount(0)
    , m_anTextIndex{}
    , m_aeTextState{}
    , m_textElapsed(DurationMs::zero())
{
	// Not SetLayoutMode() here -- CNewUIManager::AddUIObj() (called from Create(), below)
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
	// sprites/font on a second Create() call (RepositionSceneUI()'s resolution-change path).
	Release();

	CInput rInput = CInput::Instance();

	m_sprBg.Create(rInput.GetScreenWidth(), rInput.GetScreenHeight(), -1, 0, NULL, 0, 0, false);
	m_sprBg.SetAlpha(255);
	m_sprBg.SetColor(0, 0, 0);

	float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
	float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;

	m_aSpr[CRW_SPR_DECO].Create(189, 103, BITMAP_LOG_IN + 6);
	m_aSpr[CRW_SPR_LOGO].Create(290, 41, BITMAP_LOG_IN + 14, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);

	for (int i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
	{
		m_aSpr[i].Create(800, 42, -1, 0, NULL, 0, 0, false,
			SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
		m_aSpr[i].SetColor(0, 0, 0);
	}

	m_btnClose.Create(54, 30, BITMAP_BUTTON + 2, 3, 2, 1);

	int nFontSize = 10;
	switch (rInput.GetScreenWidth())
	{
	case 800:	nFontSize = 14;	break;
	case 1024:	nFontSize = 18;	break;
	case 1280:	nFontSize = 24;	break;
	}
	HFONT fontHandle = CreateFont(nFontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, I18N::Game::Gulim[0] ? I18N::Game::Gulim : NULL);
	m_font.reset(fontHandle);

	LoadText();
	SetPosition();

	// docs/rmlui-ui-system's CUIMng/CNewUIManager merger -- registers with CUIMng's own
	// scene-scoped manager instance, not the shared g_pNewUIMng (this window only ever exists
	// during LOG_IN_SCENE; see CUIMng::GetNewStyleMng()'s own comment for why). AddUIObj() is
	// already idempotent (no-ops if already registered), so this is safe to call again on every
	// RepositionSceneUI()-triggered recreate.
	CUIMng::Instance().GetNewStyleMng().AddUIObj(SEASON3B::INTERFACE_CREDITS, this);

	// Matches CWin::Create()'s own unconditional m_bShow=false reset -- RepositionSceneUI()
	// snapshots visibility before calling this and restores it right after.
	Show(false);
}

void CCreditWin::Release()
{
	m_sprBg.Release();
	for (int i = 0; i < CRW_SPR_MAX; ++i)
		m_aSpr[i].Release();
	m_font.reset();
}

void CCreditWin::SetPosition()
{
	m_aSpr[CRW_SPR_PIC_L].SetPosition(0, 126);
	m_aSpr[CRW_SPR_PIC_R].SetPosition(400, 126);
	m_aSpr[CRW_SPR_LOGO].SetPosition(241, 549);


	CInput rInput = CInput::Instance();

	int nBaseY = int(527.0f / 600.0f * (float)rInput.GetScreenHeight());
	m_aSpr[CRW_SPR_DECO].SetPosition(rInput.GetScreenWidth() - m_aSpr[CRW_SPR_DECO].GetWidth(), nBaseY - m_aSpr[CRW_SPR_DECO].GetHeight());

	for (int i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
		m_aSpr[i].SetPosition(0, 42 * (i - CRW_SPR_TXT_HIDE0));

	m_btnClose.SetPosition(m_aSpr[CRW_SPR_DECO].GetXPos() + 122,
		m_aSpr[CRW_SPR_DECO].GetYPos() + 63);
}

void CCreditWin::Show(bool bShow)
{
	SEASON3B::CNewUIObj::Show(bShow);

	m_sprBg.Show(bShow);
	for (int i = 0; i < CRW_SPR_MAX; ++i)
		m_aSpr[i].Show(bShow);

	m_btnClose.Show(bShow);

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

	// m_btnClose.Update() used to run automatically inside CWin::Update()'s own button-list step
	// (RegisterButton()) -- called explicitly now.
	m_btnClose.Update();

	if (m_btnClose.IsClick())
		CloseWin();
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		::PlayBuffer(SOUND_CLICK01);
		CloseWin();
	}

	for (int i = 0; i <= CRW_INDEX_NAME; ++i)
		AnimationText(i, deltaTime);
	AnimationIllust(deltaTime);

	return true;
}

bool CCreditWin::Render()
{
	m_sprBg.Render();

	mu::GetRenderer().SetAlphaTest(false);

	for (int i = 0; i <= CRW_SPR_LOGO; ++i)
		m_aSpr[i].Render();

	long lScreenWidth = CInput::Instance().GetScreenWidth();
	int nTextBoxWidth;

	g_pRenderText->SetFont(m_font.get());
	g_pRenderText->SetTextColor(CLRDW_BR_GRAY);
	g_pRenderText->SetBgColor(0);
	nTextBoxWidth = lScreenWidth / g_fScreenRate_x;

	auto renderCentered = [&](const SCreditItem& item, int x, int y, int width)
	{
		wchar_t buffer[CRW_NAME_MAX]{};
		CopyNameToWide(item.szName, buffer);
		g_pRenderText->RenderText(x, y, buffer, width, 0, RT3_SORT_CENTER);
	};

	renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_DEPARTMENT]], 0, 20, nTextBoxWidth);
	renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_TEAM]], 0, 46, nTextBoxWidth);

	g_pRenderText->SetTextColor(CLRDW_BR_YELLOW);

	switch (m_nNameCount)
	{
	case 1:
		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]], 0, 72, nTextBoxWidth);
		break;
	case 2:
		nTextBoxWidth = lScreenWidth / 4 / g_fScreenRate_x;
		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]], 160, 72, nTextBoxWidth);
		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME1]], 320, 72, nTextBoxWidth);
		break;
	case 3:
		nTextBoxWidth = lScreenWidth / 3 / g_fScreenRate_x;
		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]], 0, 72, nTextBoxWidth);
		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME1]], 213, 72, nTextBoxWidth);
		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME2]], 426, 72, nTextBoxWidth);
		break;
	case 4:
		nTextBoxWidth = lScreenWidth / 4 / g_fScreenRate_x;

		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]], 0, 72, nTextBoxWidth);
		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME1]], 160, 72, nTextBoxWidth);
		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME2]], 320, 72, nTextBoxWidth);
		renderCentered(m_aCredit[m_anTextIndex[CRW_INDEX_NAME3]], 480, 72, nTextBoxWidth);
		break;
	}

	for (int i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
		m_aSpr[i].Render();

	mu::GetRenderer().SetAlphaTest(true);

	m_btnClose.Render();

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
	LoadIllust();

	for (int i = 0; i <= CRW_INDEX_NAME; ++i)
		m_aeTextState[i] = FADEIN;
	m_textElapsed = DurationMs::zero();
	m_nNowIndex = 0;
	m_nNameCount = 0;
	SetTextIndex();
}

void CCreditWin::LoadIllust()
{
	CInput rInput = CInput::Instance();
	float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
	float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;

	for (int i = 0; i < 2; ++i)
	{
		const auto& illustPath = m_illustPaths[m_byIllust][i];
		LoadBitmap(illustPath, BITMAP_TEMP + i, GL_LINEAR);

		m_aSpr[i].Create(400, 400, BITMAP_TEMP + i, 0, NULL, 0, 0,
			false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
		m_aSpr[i].SetAlpha(0);
		m_aSpr[i].Show(true);
	}

	m_aSpr[CRW_SPR_PIC_L].SetPosition(0, 126);
	m_aSpr[CRW_SPR_PIC_R].SetPosition(400, 126);
}


void CCreditWin::AnimationIllust(DurationMs deltaTime)
{
	short nAlpha;
	switch (m_eIllustState)
	{
	case FADEIN:
		nAlpha = short(m_aSpr[CRW_SPR_PIC_L].GetAlpha());
		nAlpha = IncreaseAlpha(nAlpha, deltaTime / kIllustFadeDuration);
		if (255 <= nAlpha)
		{
			m_eIllustState = SHOW;
			nAlpha = 255;
		}
		m_aSpr[CRW_SPR_PIC_L].SetAlpha((BYTE)nAlpha);
		m_aSpr[CRW_SPR_PIC_R].SetAlpha((BYTE)nAlpha);
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
		nAlpha = short(m_aSpr[CRW_SPR_PIC_L].GetAlpha());
		nAlpha = DecreaseAlpha(nAlpha, deltaTime / kIllustFadeDuration);
		if (0 >= nAlpha)
		{
			m_eIllustState = FADEIN;
			nAlpha = 0;

			m_byIllust = ++m_byIllust == CRW_ILLUST_MAX ? 0 : m_byIllust;
			LoadIllust();
		}
		m_aSpr[CRW_SPR_PIC_L].SetAlpha((BYTE)nAlpha);
		m_aSpr[CRW_SPR_PIC_R].SetAlpha((BYTE)nAlpha);
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
	short nAlpha;

	CSprite* psprHide = &m_aSpr[CRW_SPR_TXT_HIDE0 + nClass];

	switch (*peTextState)
	{
	case FADEIN:
		nAlpha = short(psprHide->GetAlpha());
		nAlpha = DecreaseAlpha(nAlpha, deltaTime / kTextFadeDuration);
		if (0 >= nAlpha)
		{
			*peTextState = SHOW;
			nAlpha = 0;
		}
		psprHide->SetAlpha((BYTE)nAlpha);
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
		nAlpha = short(psprHide->GetAlpha());
		nAlpha = IncreaseAlpha(nAlpha, deltaTime / kTextFadeDuration);
		if (255 <= nAlpha)
		{
			*peTextState = FADEIN;
			nAlpha = 255;

			if (nClass == CRW_INDEX_NAME)
				SetTextIndex();
		}
		psprHide->SetAlpha((BYTE)nAlpha);
		break;
	}
}
