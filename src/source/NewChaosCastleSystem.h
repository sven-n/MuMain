// NewChaosCastleSystem.h: interface for the CNewUIChaosCastleSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWCHAOSCASTLESYSTEM_H__C7E9AF82_7FEB_453F_8A8C_4657FD7483BD__INCLUDED_)
#define AFX_NEWCHAOSCASTLESYSTEM_H__C7E9AF82_7FEB_453F_8A8C_4657FD7483BD__INCLUDED_

#pragma once

#include "CSEventMatch.h"

namespace SEASON3B
{
    class CNewChaosCastleSystem : public CSBaseMatch
    {
    private:
        int     m_iChaosCastleLimitArea1[16];
        int     m_iChaosCastleLimitArea2[16];
        int     m_iChaosCastleLimitArea3[16];
        BYTE	m_byCurrCastleLevel;
        bool    m_bActionMatch;

    public:
        CNewChaosCastleSystem();
        virtual ~CNewChaosCastleSystem();

        virtual void    RenderMatchTimes(void);

        virtual void    SetMatchGameCommand(const LPPRECEIVE_MATCH_GAME_STATE data);
        virtual void    SetMatchResult(const int iNumDevil, const int iMyRank, const MatchResult* pMatchResult, const int Success = false);
        virtual void    RenderMatchResult(void);
    };
};

#endif // !defined(AFX_NEWCHAOSCASTLESYSTEM_H__C7E9AF82_7FEB_453F_8A8C_4657FD7483BD__INCLUDED_)
