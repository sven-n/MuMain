// CNewUISiegeWarfare.h: interface for the CNewUISiegeWarfare class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CNewUISiegeWarfare_H__6810678B_808B_4765_B9AC_AC34344E7E2D__INCLUDED_)
#define AFX_CNewUISiegeWarfare_H__6810678B_808B_4765_B9AC_AC34344E7E2D__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "_struct.h"
#include "NewUIManager.h"
#include "NewUISiegeWarBase.h"

namespace SEASON3B
{
    class CNewUISiegeWarfare : public CNewUIObj
    {
    public:
        enum SIEGEWAR_TYPE
        {
            SIEGEWAR_TYPE_NONE = -1,
            SIEGEWAR_TYPE_OBSERVER = 0,
            SIEGEWAR_TYPE_COMMANDER,
            SIEGEWAR_TYPE_SOLDIER,
        };

    private:
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;

        CNewUISiegeWarBase* m_pSiegeWarUI;
        short						m_sGuildMarkIndex;
        BYTE						m_byGuildStatus;
        int							m_iCurSiegeWarType;

        int							m_iHour;
        int							m_iMinute;
        int							m_iSecond;
        DWORD						m_dwSyncTime;

        bool						m_bCreated;

    public:
        CNewUISiegeWarfare();
        virtual ~CNewUISiegeWarfare();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        bool CreateMiniMapUI();
        void InitMiniMapUI();
        void SetGuildData(const CHARACTER* pCharacter);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 1.6f

        void OpenningProcess();
        void ClosingProcess();

        void ClearGuildMemberLocation(void);
        void SetGuildMemberLocation(BYTE type, int x, int y);
        void SetTime(BYTE byHour, BYTE byMinute);

        void SetMapInfo(GuildCommander& data);

    public:
        inline CNewUISiegeWarBase* GetBase()
        {
            if (!m_pSiegeWarUI)
                return NULL;

            return m_pSiegeWarUI;
        }

        inline int GetCurSiegeWarType() { return m_iCurSiegeWarType; };
        inline bool IsCreated() { return m_bCreated; };

        void InitSkillUI();
        void ReleaseSkillUI();
    };
}

#endif // !defined(AFX_CNewUISiegeWarfare_H__6810678B_808B_4765_B9AC_AC34344E7E2D__INCLUDED_)