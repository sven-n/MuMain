#include "stdafx.h"
#include "UI/HUD/SlideTicker.h"

#include "Core/Time/FrameTimerScheduler.h"
#include "Render/Text/CUIRenderText.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Options/OptionWindow.h"
#include "UI/Core/WindowSystem.h"

// Defined in UIWindows.cpp; declared per translation unit across the UI, with no header.
void SetLineColor(int iType, float fAlphaRate = 1.0f);

namespace UI::HUD
{
SlideLane::SlideLane()
{
    m_fMaxMoveSpeed = 2.5f;
    m_dwSlideTextColor = 0;
    m_dwTimer = 0;
    m_hFont = nullptr;
    m_bBlink = FALSE;
}

SlideLane::~SlideLane()
{
}

void SlideLane::Init(BOOL bBold, BOOL bBlink)
{
    if (bBold == TRUE)
    {
        m_hFont = g_hFontBold;
    }
    else
    {
        m_hFont = g_hFont;
    }

    m_bBlink = bBlink;

    m_dwCurrentSecond = 10;
    m_dwTimer = timeGetTime();

    memset(m_szSlideTextA, 0, SLIDE_TEXT_LENGTH);
    memset(m_szSlideTextB, 0, SLIDE_TEXT_LENGTH);

    m_fMovePosition = (float)REFERENCE_WIDTH;
    m_fMoveAccel = 1.0f;
    m_fMoveSpeed = 0;
    m_iCutLength = 0;
    m_iCutSize = 0;
    m_pszSlideText = m_szSlideTextA;
    m_iFontHeight = 0;
    m_iAlphaRate = 0;
}

BOOL SlideLane::DoMouseAction()
{
    return FALSE;
}

BOOL SlideLane::HaveText()
{
    return (m_pszSlideText[0] == '\0' || (int)wcslen(m_pszSlideText) < m_iCutLength);
}

int g_iNoticeInverse = 0;

void SlideLane::Render(BOOL bForceFadeOut)
{
    if (g_pOption->IsSlideHelp() == false)
    {
        return;
    }

    BOOL bFadeOut = FALSE;

    if (m_pszSlideText[0] == '\0' || (int)wcslen(m_pszSlideText) < m_iCutLength)
    {
        bFadeOut = TRUE;
    }
    else if (bForceFadeOut == TRUE)
    {
        bFadeOut = TRUE;
    }

    if (bFadeOut == FALSE)
    {
        m_iAlphaRate += 30.f * FPS_ANIMATION_FACTOR;
        m_iAlphaRate = std::min<float>(m_iAlphaRate, 205.f);
    }
    else
    {
        m_iAlphaRate -= 30 * FPS_ANIMATION_FACTOR;
        m_iAlphaRate = std::max<float>(m_iAlphaRate, 0.f);
    }

    if (m_iAlphaRate <= 0)
    {
        return;
    }

    EnableAlphaTest();

    // The band never set its own colour: it drew in whatever SetRenderColor state the last widget
    // to render had left, which happened to be dark while the CUIControl list boxes were still
    // drawing. Retiring them left it at the default and the band came out opaque white. It states
    // its own colour now, black at the lane's current alpha so it fades with the text it carries.
    SetLineColor(7, m_iAlphaRate / 255.f);

    RenderColor(0, m_iPos_y - 3, WindowWidth, 1);
    RenderColor(0, m_iPos_y + m_iFontHeight + 2, WindowWidth, 1);
    RenderColor(0, m_iPos_y - 2, WindowWidth, m_iFontHeight + 4);

    EndRenderColor();

    if (bFadeOut == TRUE && bForceFadeOut == FALSE)
    {
        DisableAlphaBlend();
        return;
    }

    g_pRenderText->SetFont(m_hFont);
    SlideMove();

    if (m_iFontHeight == 0)
    {
        m_iFontHeight = g_pRenderText->MeasureText(L"Z", 1).cy;

        if (GetPosition_y() >= m_iFontHeight)
        {
            SetPosition(GetPosition_x(), GetPosition_y() - m_iFontHeight);
        }
    }

    g_pRenderText->SetTextColor(m_dwSlideTextColor & 0x00FFFFFF);

    BYTE byAlpha = m_dwSlideTextColor >> 24;
    byAlpha = static_cast<float>(byAlpha) * ((m_iAlphaRate > 180 ? m_iAlphaRate : (m_iAlphaRate - 25 < 0 ? 0 : m_iAlphaRate - 25)) + 50) / 255.0f;
    if (const auto frac = WorldTime - static_cast<long>(WorldTime);
        m_bBlink == TRUE && frac < 0.5)
    {
        byAlpha /= 2;
    }

    g_pRenderText->SetTextColor(g_pRenderText->GetTextColor() + (byAlpha << 24));
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(static_cast<int>(m_fMovePosition), m_iPos_y, m_pszSlideText);
    DisableAlphaBlend();

    ComputeSpeed();

    ++g_iNoticeInverse;
}

void SlideLane::SlideMove()
{
    if (m_iCutSize == 0)
    {
        m_iCutSize = CheckCutSize(m_pszSlideText, 4);
    }

    if (m_fMovePosition < m_iCutSize * -1)
    {
        memset(m_pszSlideText, 0, SLIDE_TEXT_LENGTH);
        m_fMovePosition = (float)REFERENCE_WIDTH;
        m_iCutSize = CheckCutSize(m_pszSlideText, 4);
    }
}

void SlideLane::ComputeSpeed()
{
    if (mu::ui::window::CheckMouseIn(0, m_iPos_y - 3, WindowWidth, m_iFontHeight + 6) == FALSE)
    {
        m_fMoveAccel = 1.0f;
    }
    else
    {
        m_fMoveAccel = -1.0f;
    }

    m_fMoveSpeed += m_fMoveAccel * FPS_ANIMATION_FACTOR;
    if (m_fMoveSpeed >= m_fMaxMoveSpeed)
    {
        m_fMoveSpeed = m_fMaxMoveSpeed;
    }
    else if (m_fMoveSpeed < 0)
    {
        m_fMoveSpeed = 0;
    }

    m_fMovePosition -= m_fMoveSpeed * FPS_ANIMATION_FACTOR;
}

BOOL SlideLane::AddSlideText(const wchar_t* pszNewText, DWORD dwTextColor)
{
    if (pszNewText == nullptr || pszNewText[0] == '\0') return TRUE;

    m_dwSlideTextColor = dwTextColor;
    g_pRenderText->SetFont(m_hFont);
    wcscat(m_pszSlideText, pszNewText);
    return TRUE;
}

int SlideLane::CheckCutSize(const wchar_t* pszSource, int iNeedValue)
{
    if (pszSource == nullptr || pszSource[0] == L'\0') return 0;

    auto iLength = wcslen(pszSource);
    int iMove = 2; // might be 4, too
    int iTextSize = 0;
    for (int i = 0; i < iLength; )
    {
        iTextSize = i;
        if (i + iMove > iNeedValue)
        {
            break;
        }
        else
        {
            i += iMove;
        }
    }
    m_iCutLength = iTextSize;

    return g_pRenderText->MeasureText(pszSource, m_iCutLength).cx;
}

void SlideLane::SetScrollSpeed(float fSpeed)
{
    if (fSpeed <= 0)
    {
        fSpeed = 2.5f;
    }

    m_fMaxMoveSpeed = fSpeed * g_fScreenRate_x;
}

void SlideLane::AddSlide(int iLoopCount, int iLoopDelay, const wchar_t* pszText, int iType, float fSpeed, DWORD dwTextColor)
{
    if (SceneFlag != MAIN_SCENE) return;
    if (pszText == nullptr || pszText[0] == '\0') return;
    if (iLoopCount > 30) return;
    int iLength = wcslen(pszText);

    SLIDE_QUEUE_DATA slidedata;
    slidedata.m_iType = iType;
    slidedata.m_pszText = new wchar_t[iLength + 1];
    memset(slidedata.m_pszText, 0, iLength + 1);
    wcsncpy(slidedata.m_pszText, pszText, iLength + 1);
    slidedata.m_fSpeed = fSpeed;
    slidedata.m_dwTextColor = dwTextColor;
    slidedata.m_bLastData = FALSE;

    for (int i = 0; i < iLoopCount; ++i)
    {
        if (i + 1 == iLoopCount)
        {
            slidedata.m_bLastData = TRUE;
        }

        if (iType == 1)
        {
            m_SlideQueue.insert(std::pair<DWORD, SLIDE_QUEUE_DATA>(0, slidedata));
        }
        else
        {
            m_SlideQueue.insert(std::pair<DWORD, SLIDE_QUEUE_DATA>(m_dwCurrentSecond + i * iLoopDelay, slidedata));
        }
    }
}

void SlideLane::CheckTime()
{
    DWORD dwCheckTime = timeGetTime();
    if (dwCheckTime > m_dwTimer + 1000)
    {
        m_dwTimer = timeGetTime();
        ++m_dwCurrentSecond;
    }
}

void SlideLane::ManageSlide()
{
    BOOL bFadeOut = FALSE;

    if (m_pszSlideText[0] == '\0' || (int)wcslen(m_pszSlideText) < m_iCutLength)
    {
        bFadeOut = TRUE;
    }
    if (bFadeOut == FALSE)
    {
        return;
    }

    CheckTime();
    for (m_SlideQueueIter = m_SlideQueue.begin(); m_SlideQueueIter != m_SlideQueue.end(); ++m_SlideQueueIter)
    {
        if (m_SlideQueueIter->first > m_dwCurrentSecond) break;
        else
        {
            if (m_SlideQueueIter->second.m_iType == -1 && m_SlideQueueIter->first + 60 < m_dwCurrentSecond);
            else if (AddSlideText(m_SlideQueueIter->second.m_pszText, m_SlideQueueIter->second.m_dwTextColor) == FALSE) break;

            SetScrollSpeed(m_SlideQueueIter->second.m_fSpeed);
            if (m_SlideQueueIter->second.m_bLastData == TRUE)
            {
                delete[] m_SlideQueueIter->second.m_pszText;
                m_SlideQueueIter->second.m_pszText = nullptr;
            }
            m_SlideQueue.erase(m_SlideQueueIter);
            break;
        }
    }
}

SlideTicker::SlideTicker()
{
    m_iCreateDelay = 10;
    m_fHelpSlideSpeed = 2.5f;
}

SlideTicker::~SlideTicker()
{
    // The timer callback captures this; kill it so it can't fire on a destroyed instance.
    if (auto* scheduler = Core::Time::FrameTimerScheduler::TryInstance())
    {
        scheduler->Kill(SLIDEHELP_TIMER);
    }
    ClearSlideText();
}

void SlideTicker::Init()
{
    m_HelpSlide.Init();
    m_NoticeSlide.Init(TRUE, FALSE);

    m_HelpSlide.SetPosition(0, 0);
    m_NoticeSlide.SetPosition(0, 0);
    //m_HelpSlide.SetPosition(0, 429);
    //m_NoticeSlide.SetPosition(0, 429);

    extern bool g_bWndActive;
    Core::Time::FrameTimerScheduler::Instance().SetRepeating(
        SLIDEHELP_TIMER, m_iCreateDelay * 1000,
        [this] { if (g_bWndActive) CreateSlideText(); });
    CreateSlideText();
}

void SlideTicker::Render()
{
    if (m_NoticeSlide.HaveText() == FALSE)
    {
        m_HelpSlide.Render(TRUE);
    }
    else if (m_NoticeSlide.GetAlphaRate() <= 0)
    {
        m_HelpSlide.Render();
    }

    if (m_HelpSlide.GetAlphaRate() <= 0)
    {
        m_NoticeSlide.Render();
    }
}

void SlideTicker::CreateSlideText()
{
    if (SceneFlag != MAIN_SCENE)
        return;
    if (g_pOption->IsSlideHelp() == false)
    {
        return;
    }
    if (m_HelpSlide.GetAlphaRate() > 0)
        return;

    int iLevel = CharacterMachine->Character.Level;

    const wchar_t* pszNewText = GetSlideText(iLevel);

    AddSlide(1, 0, pszNewText, 1, m_fHelpSlideSpeed);
}

void SlideTicker::OpenSlideTextFile(const wchar_t* szFileName)
{
    for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
    {
        if (!m_SlideTextList[i].empty())
        {
            ClearSlideText();
            break;
        }
    }

    FILE* fp = _wfopen(szFileName, L"rb");
    if (fp == nullptr)
    {
        wchar_t Text[256];
        mu_swprintf(Text, L"%ls - File not exist.", szFileName);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    SLIDEHELP SlideHelp;
    fread(&SlideHelp, sizeof(SLIDEHELP), 1, fp);
    BuxConvert((BYTE*)&SlideHelp, sizeof(SLIDEHELP));
    fclose(fp);

    SetCreateDelay(SlideHelp.iCreateDelay);
    m_fHelpSlideSpeed = SlideHelp.fSpeed;

    for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
    {
        m_iLevelCap[i] = SlideHelp.SlideHelp[i].iLevel;
        m_iTextNumber[i] = SlideHelp.SlideHelp[i].iNumber;
        for (int j = 0; j < m_iTextNumber[i]; ++j)
        {
            auto charText = SlideHelp.SlideHelp[i].szSlideHelpText[j];
            int iLength = MultiByteToWideChar(CP_UTF8, 0, charText, -1, 0, 0);
            auto pszText = new wchar_t[iLength + 1];
            MultiByteToWideChar(CP_UTF8, 0, charText, -1, pszText, iLength);
            m_SlideTextList[i].push_back(pszText);
        }
    }
}

void SlideTicker::ClearSlideText()
{
    for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
    {
        m_iLevelCap[i] = 0;
        m_iTextNumber[i] = 0;
        for (m_SlideTextListIter = m_SlideTextList[i].begin(); m_SlideTextListIter != m_SlideTextList[i].end(); ++m_SlideTextListIter)
        {
            if (*m_SlideTextListIter != nullptr)
            {
                delete[] * m_SlideTextListIter;
                *m_SlideTextListIter = nullptr;
            }
        }
        m_SlideTextList[i].clear();
    }
}

const wchar_t* SlideTicker::GetSlideText(int iLevel)
{
    int iHelpType = -1;
    for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
    {
        if (iLevel <= m_iLevelCap[i])
        {
            iHelpType = i;
            break;
        }
    }
    if (iHelpType == -1) return nullptr;
    if (m_iTextNumber[iHelpType] == 0) return nullptr;

    int iRandom = rand() % m_iTextNumber[iHelpType];

    if ((unsigned int)iRandom >= m_SlideTextList[iHelpType].size()) return nullptr;

    m_SlideTextListIter = m_SlideTextList[iHelpType].begin();
    for (int i = 0; i < iRandom; ++i)
        ++m_SlideTextListIter;

    return *m_SlideTextListIter;
}

void SlideTicker::AddSlide(int iLoopCount, int iLoopDelay, const wchar_t* pszText, int iType, float fSpeed, DWORD dwTextColor)
{
    if (SceneFlag != MAIN_SCENE) return;
    if (pszText == nullptr || pszText[0] == '\0') return;
    if (iLoopCount > 30) return;

    switch (iType)
    {
    case 2:	case 1:	case 0:
        m_HelpSlide.AddSlide(iLoopCount, iLoopDelay, pszText, iType - 1, fSpeed, dwTextColor);
        break;
    case 5:	case 4:	case 3:
        m_NoticeSlide.AddSlide(iLoopCount, iLoopDelay, pszText, iType - 4, fSpeed, dwTextColor);
        break;
    default:
        break;
    };
}

void SlideTicker::ManageSlide()
{
    m_NoticeSlide.ManageSlide();
    m_HelpSlide.ManageSlide();
}

BOOL SlideTicker::IsIdle()
{
    return (m_NoticeSlide.HaveText() && m_HelpSlide.HaveText());
}
} // namespace UI::HUD
