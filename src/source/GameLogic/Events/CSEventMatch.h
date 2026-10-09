//////////////////////////////////////////////////////////////////////////
//  CSEventMatch.h
//////////////////////////////////////////////////////////////////////////
#ifndef __CSEVENT_MATCH_H__
#define __CSEVENT_MATCH_H__

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "Network/Server/WSclient.h"

using MatchClock = std::chrono::steady_clock;

// One text of an event's result box, as RenderMatchResult() draws it: RenderText(x, y, text,
// boxWidth, boxHeight, sort) in reference units, in its font (or the one left set) and colour.
struct MatchResultText
{
    enum class Font
    {
        Unchanged,
        Normal,
        Bold,
    };

    std::wstring text;
    int x = 0;
    int y = 0;
    int boxWidth = 0;
    int boxHeight = 0;
    int sort = RT3_SORT_LEFT;
    Font font = Font::Unchanged;
    DWORD color = 0; // RGBA()
};

// Draws the texts natively (g_pRenderText), in order.
void RenderMatchResultTexts(const std::vector<MatchResultText>& texts);

class CSBaseMatch
{
protected:
    std::uint8_t m_byMatchEventType;
    MATCH_TYPE  m_iMatchCountDownType;
    MatchClock::time_point m_matchCountDownStart;
    std::uint8_t m_byMatchType;
    int         m_iMatchTimeMax;
    int         m_iMatchTime;

    int         m_iMaxKillMonster;
    int         m_iKillMonster;

    int         m_iNumResult;
    int         m_iMyResult;
    MatchResult m_MatchResult[11];

    // As clearMatchInfo() sets it: a result before any state is placed as in a running event.
    POINT m_PosResult{REFERENCE_WIDTH - 230 / 2, 100};

    bool    getEqualMonster(int addV);


public:
    CSBaseMatch()
        : m_byMatchEventType(0)
        , m_iMatchCountDownType(TYPE_MATCH_NONE)
        , m_matchCountDownStart(MatchClock::time_point{})
        , m_byMatchType(0)
        , m_iMatchTimeMax(-1)
        , m_iMatchTime(-1)
        , m_iMaxKillMonster(-1)
        , m_iKillMonster(-1)
        , m_iNumResult(0)
        , m_iMyResult(0)
        , m_MatchResult{}
    {
    }
    virtual ~CSBaseMatch() {};

    void    clearMatchInfo(void);
    std::uint8_t GetMatchEventType(void) { return m_byMatchEventType; }

    std::uint8_t GetMatchType() { return m_byMatchType; }
    int		GetMatchTime() { return m_iMatchTime; }
    int		GetMatchMaxTime() { return m_iMatchTimeMax; };
    int		GetNumMustKillMonster() { return m_iMaxKillMonster; }
    int		GetNumKillMonster() { return m_iKillMonster; }

    void	SetPosition(int ix, int iy);
    void    StartMatchCountDown(int iType);
    void    SetMatchInfo(std::uint8_t byType, int iMaxTime, int iTime, int iMaxMonster = 0, int iKillMonster = 0);

    // The entry countdown's line while one runs (30 seconds from StartMatchCountDown()), else empty.
    std::wstring CountdownText(void);
    virtual void    RenderMatchTimes(void) = 0;

    virtual void    SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data) = 0;
    virtual void    SetMatchResult(const int iNumDevilRank, const int iMyRank, const MatchResult* pMatchResult, const int Success = false) = 0;
    virtual void    RenderMatchResult(void) = 0;
    // The texts RenderMatchResult() draws (none for an event without a result box).
    virtual void CollectMatchResult(std::vector<MatchResultText>& texts) const {}
};

class CSDevilSquareMatch : public CSBaseMatch
{
private:

public:
    CSDevilSquareMatch() {};
    virtual ~CSDevilSquareMatch() {};

    virtual void    RenderMatchTimes(void);

    virtual void    SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data);
    virtual void    SetMatchResult(const int iNumDevilRank, const int iMyRank, const MatchResult* pMatchResult, const int Success = false);
    virtual void    RenderMatchResult(void);
    virtual void CollectMatchResult(std::vector<MatchResultText>& texts) const;
};

class CCursedTempleMatch : public CSBaseMatch
{
private:

public:
    CCursedTempleMatch() {};
    virtual ~CCursedTempleMatch() {};

    virtual void    RenderMatchTimes(void);

    virtual void    SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data);
    virtual void    SetMatchResult(const int iNumDevilRank, const int iMyRank, const MatchResult* pMatchResult, const int Success = false);
    virtual void    RenderMatchResult(void);
};

class CDoppelGangerMatch : public CSBaseMatch
{
private:

public:
    CDoppelGangerMatch() {};
    virtual ~CDoppelGangerMatch() {};

    virtual void    RenderMatchTimes(void) {}

    virtual void    SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data) {}
    virtual void    SetMatchResult(const int iNumDevilRank, const int iMyRank, const MatchResult* pMatchResult, const int Success = false) {}
    virtual void    RenderMatchResult(void) {}
};

#endif// __CSEVENT_MATCH_H__
