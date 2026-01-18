// NewUIChaosCastleTime.h: interface for the CNewUIChaosCastleTime class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICHAOSCASTLETIME_H__96004B21_205A_40BE_9B6B_3C099D8E41F6__INCLUDED_)
#define AFX_NEWUICHAOSCASTLETIME_H__96004B21_205A_40BE_9B6B_3C099D8E41F6__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIBloodCastleTime.h"

namespace SEASON3B
{
    class CNewUIChaosCastleTime : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_CHAOSCASTLE_TIME_WINDOW = CNewUIBloodCastle::IMAGE_BLOODCASTLE_TIME_WINDOW	// newui_Figure_blood.tga (124, 81)
        };

    private:
        enum BLOODCASTLE_TIME_WINDOW_SIZE
        {
            CHAOSCASTLE_TIME_WINDOW_WIDTH = 124,
            CHAOSCASTLE_TIME_WINDOW_HEIGHT = 81,
        };

        enum
        {
            CC_TIME_STATE_NORMAL = 1,
            CC_TIME_STATE_IMMINENCE,
        };

        enum
        {
            MAX_KILL_MONSTER = 65535,
        };

    private:
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;

        wchar_t				m_szTime[256];		// 시간
        int							m_iTime;			// 시간
        int							m_iTimeState;		// 시간상태( 기본, 임박 )
        int							m_iMaxKillMonster;	// 죽여야하는 몬스터숫자
        int							m_iKilledMonster;	// 현재 죽인 몬스터숫자

    public:
        CNewUIChaosCastleTime();
        virtual ~CNewUIChaosCastleTime();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 1.3f

        void OpenningProcess();
        void ClosingProcess();

    private:
        void LoadImages();
        void UnloadImages();

    public:
        void SetTime(int m_iTime);
        void SetKillMonsterStatue(int iKilled, int iMaxKill);
    };
};

#endif // !defined(AFX_NEWUICHAOSCASTLETIME_H__96004B21_205A_40BE_9B6B_3C099D8E41F6__INCLUDED_)
