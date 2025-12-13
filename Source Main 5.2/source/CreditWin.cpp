//*****************************************************************************
// File: CreditWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CreditWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "DSPlaySound.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "Local.h"

#include "UIControls.h"

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
}

CCreditWin::~CCreditWin()
{

}

void CCreditWin::Create()
{
	CInput rInput = CInput::Instance();

	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());
	CWin::SetBgAlpha(255);

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
	CWin::RegisterButton(&m_btnClose);

	int nFontSize = 10;
	switch (rInput.GetScreenWidth())
	{
	case 800:	nFontSize = 14;	break;
	case 1024:	nFontSize = 18;	break;
	case 1280:	nFontSize = 24;	break;
	}
	HFONT fontHandle = CreateFont(nFontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, GlobalText[0][0] ? GlobalText[0] : NULL);
	m_font.reset(fontHandle);

	LoadText();
	SetPosition();
}

void CCreditWin::PreRelease()
{
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
	CWin::Show(bShow);

	for (int i = 0; i < CRW_SPR_MAX; ++i)
		m_aSpr[i].Show(bShow);

	m_btnClose.Show(bShow);

	if (bShow)
		Init();
	else
		m_eIllustState = HIDE;
}

bool CCreditWin::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)
		return false;

	switch (nArea)
	{
	case WA_MOVE:
		return false;
	}

	return CWin::CursorInWin(nArea);
}

void CCreditWin::UpdateWhileActive(double deltaMilliseconds)
{
	const DurationMs deltaTime{ deltaMilliseconds };

	if (m_btnClose.IsClick())
		CloseWin();
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		::PlayBuffer(SOUND_CLICK01);
		CloseWin();
		CUIMng::Instance().SetSysMenuWinShow(false);
	}

	for (int i = 0; i <= CRW_INDEX_NAME; ++i)
		AnimationText(i, deltaTime);
	AnimationIllust(deltaTime);
}

void CCreditWin::RenderControls()
{
	::glDisable(GL_ALPHA_TEST);

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

	glEnable(GL_ALPHA_TEST);

	CWin::RenderButtons();
}

void CCreditWin::CloseWin()
{
	CUIMng::Instance().HideWin(this);

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
	std::unique_ptr<FILE, decltype(&std::fclose)> file(std::fopen(kCreditDataPath.data(), "rb"), &std::fclose);
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
