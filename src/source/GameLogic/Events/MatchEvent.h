//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

#ifndef __MATCH_EVENT_H__
#define __MATCH_EVENT_H__

/////////////////////////////////////////////////////////////////////////
//  INCLUDE.
//////////////////////////////////////////////////////////////////////////
#include "CSEventMatch.h"
#include "CSChaosCastle.h"
#include "NewChaosCastleSystem.h"
#include "NewBloodCastleSystem.h"

//////////////////////////////////////////////////////////////////////////
//  NAMESPACE
//////////////////////////////////////////////////////////////////////////
namespace matchEvent
{
    extern  CSBaseMatch* g_csMatchInfo;

    void    CreateEventMatch(int iWorld);
    void    DeleteEventMatch();

    inline  void    StartMatchCountDown(int iType)
    {
        if (g_csMatchInfo != NULL)
        {
            g_csMatchInfo->StartMatchCountDown(iType);
        }
    }
    inline  void    ClearMatchInfo(void)
    {
        if (g_csMatchInfo != NULL)
        {
            g_csMatchInfo->clearMatchInfo();
        }
    }
    inline  void    SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data)
    {
        if (g_csMatchInfo != NULL)
        {
            g_csMatchInfo->SetMatchGameCommand(data);
        }
    }
    inline  std::wstring CountdownText(void)
    {
        if (g_csMatchInfo != NULL)
        {
            return g_csMatchInfo->CountdownText();
        }
        return {};
    }
    inline  void    RenderMatchTimes(void)
    {
        if (g_csMatchInfo != NULL)
        {
            g_csMatchInfo->RenderMatchTimes();
        }
    }
    inline  void	SetMatchResult(int iNumResult, int iMyResult, MatchResult* pResult, int Success = 0)
    {
        if (g_csMatchInfo != NULL)
        {
            g_csMatchInfo->SetMatchResult(iNumResult, iMyResult, pResult, Success);
        }
    }
    inline void CollectResult(std::vector<MatchResultLine>& lines)
    {
        if (g_csMatchInfo != NULL)
        {
            g_csMatchInfo->CollectMatchResult(lines);
        }
    }
}

using namespace matchEvent;

#endif// __MATCH_EVENT_H__
