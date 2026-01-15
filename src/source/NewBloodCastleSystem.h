// NewBloodCastleSystem.h: interface for the CNewBloodCastleSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWBLOODCASTLESYSTEM_H__8A912BDC_03C6_4458_9704_46C6E4E7D915__INCLUDED_)
#define AFX_NEWBLOODCASTLESYSTEM_H__8A912BDC_03C6_4458_9704_46C6E4E7D915__INCLUDED_

#pragma once

#include "CSEventMatch.h"

namespace SEASON3B
{
    class CNewBloodCastleSystem : public CSBaseMatch
    {
    public:
        CNewBloodCastleSystem();
        virtual ~CNewBloodCastleSystem();

        virtual void    RenderMatchTimes(void);
        virtual void    SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data);
        virtual void    SetMatchResult(const int iNumDevil, const int iMyRank, const MatchResult* pMatchResult, const int Success = false);
        virtual void    RenderMatchResult(void);
    };
};

#endif // !defined(AFX_NEWBLOODCASTLESYSTEM_H__8A912BDC_03C6_4458_9704_46C6E4E7D915__INCLUDED_)
