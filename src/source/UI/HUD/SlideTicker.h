#pragma once

// The scrolling notice ticker: the help line driven from the local Slide_<lang>.bmd text, and the
// server notice line WSclient pushes through AddSlide(). Extracted from the widget toolkit
// unchanged -- it was a CUIControl only for the position the base carried, which SlideLane now
// keeps itself.

#include "Core/Time/Timer.h"

#include <list>
#include <map>

namespace UI::HUD
{
// What the ticker wants drawn this frame. The band is one strip: the two 1-unit rules and the
// fill Render() drew are contiguous, so they are a single box from y - 3 of height height + 6.
struct SlideDisplay
{
    const wchar_t* text = nullptr;
    float x = 0.f;        // reference px; the scroll position
    int y = 0;            // reference px, the text baseline row
    int bandHeight = 0;   // reference px
    unsigned colorRgb = 0;
    unsigned char alpha = 0;
    bool shown = false;
};

#define SLIDE_LEVEL_MAX 5
#define SLIDE_TEXT_LENGTH 1024

struct SLIDEHELPTEXT
{
    int iLevel;
    int iNumber;
    char szSlideHelpText[32][256];
};

struct SLIDEHELP
{
    int iCreateDelay;
    float fSpeed;
    SLIDEHELPTEXT SlideHelp[SLIDE_LEVEL_MAX];
};

struct SLIDE_QUEUE_DATA
{
    int m_iType;
    wchar_t* m_pszText;
    DWORD m_dwTextColor;
    float m_fSpeed;
    BOOL m_bLastData;
};

typedef std::multimap<DWORD, SLIDE_QUEUE_DATA, std::less<DWORD>> SLIDE_QUEUE;

class SlideLane
{
public:
    SlideLane();
    virtual ~SlideLane();

    void Init(BOOL bBold = FALSE, BOOL bBlink = FALSE);
    BOOL DoMouseAction();
    void Render(BOOL bForceFadeOut = FALSE);
    void SetScrollSpeed(float fSpeed);
    BOOL AddSlideText(const wchar_t* pszNewText, DWORD dwTextColor = (255u << 24) + (200u << 16) + (220u << 8) + 230u);

    int GetAlphaRate()
    {
        return m_iAlphaRate;
    }
    BOOL HaveText();

    void AddSlide(int iLoopCount, int iLoopDelay, const wchar_t* pszText, int iType, float fSpeed, DWORD dwTextColor);
    void CheckTime();
    void ManageSlide();
    DWORD m_dwTimer;
    DWORD m_dwCurrentSecond;

    SLIDE_QUEUE m_SlideQueue;
    SLIDE_QUEUE::iterator m_SlideQueueIter;
    std::list<DWORD> m_RemoveQueueList;

    // What Render() would draw, without drawing it.
    SlideDisplay Display(BOOL bForceFadeOut = FALSE) const;
    void Advance(BOOL bForceFadeOut = FALSE);

    void SetPosition(int x, int y) { m_iPos_x = x; m_iPos_y = y; }
    int GetPosition_x() const { return m_iPos_x; }
    int GetPosition_y() const { return m_iPos_y; }

protected:
    void SlideMove();
    void ComputeSpeed();

    int CheckCutSize(const wchar_t* pszSource, int iNeedValue);

protected:
    int m_iPos_x = 0;
    int m_iPos_y = 0;
    HFONT m_hFont;
    wchar_t* m_pszSlideText;
    wchar_t m_szSlideTextA[SLIDE_TEXT_LENGTH];
    wchar_t m_szSlideTextB[SLIDE_TEXT_LENGTH];
    float m_fMovePosition;
    float m_fMoveAccel;
    float m_fMoveSpeed;
    float m_fMaxMoveSpeed;
    float m_iAlphaRate;
    int m_iCutLength;
    int m_iCutSize;
    int m_iFontHeight;
    DWORD m_dwSlideTextColor;
    BOOL m_bBlink;
};

class SlideTicker
{
public:
    SlideTicker();
    virtual ~SlideTicker();

    void Init();
    void Render();
    SlideDisplay Display();

    void CreateSlideText();
    void OpenSlideTextFile(const wchar_t* szFileName);
    void ClearSlideText();
    const wchar_t* GetSlideText(int iLevel);
    void SetCreateDelay(int iDelay)
    {
        m_iCreateDelay = iDelay;
    }

    void AddSlide(int iLoopCount, int iLoopDelay, const wchar_t* pszText, int iType, float fSpeed,
                  DWORD dwTextColor = (255u << 24) + (200u << 16) + (220u << 8) + 230u);
    void ManageSlide();
    BOOL IsIdle();

protected:
    SlideLane m_HelpSlide;
    SlideLane m_NoticeSlide;

    int m_iCreateDelay;
    int m_iLevelCap[SLIDE_LEVEL_MAX];
    int m_iTextNumber[SLIDE_LEVEL_MAX];
    std::list<wchar_t*> m_SlideTextList[SLIDE_LEVEL_MAX];
    std::list<wchar_t*>::iterator m_SlideTextListIter;
    float m_fHelpSlideSpeed;
};
} // namespace UI::HUD
