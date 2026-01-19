// NewUIDoppelGangerFrame.h: interface for the CNewUIDoppelGangerFrame class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"

namespace SEASON3B
{
    class CNewUIDoppelGangerFrame : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_DOPPELGANGER_FRAME_WINDOW = BITMAP_DOPPELGANGER_FRAME_BEGIN,
            IMAGE_DOPPELGANGER_GUAGE_RED,
            IMAGE_DOPPELGANGER_GUAGE_ORANGE,
            IMAGE_DOPPELGANGER_GUAGE_YELLOW,
            IMAGE_DOPPELGANGER_GUAGE_PLAYER,
            IMAGE_DOPPELGANGER_GUAGE_PARTY_MEMBER,
            IMAGE_DOPPELGANGER_GUAGE_ICEWALKER,
        };

    private:
        enum DOPPELGANGER_FRAME_WINDOW_SIZE
        {
            DOPPELGANGER_FRAME_WINDOW_WIDTH = 227,
            DOPPELGANGER_FRAME_WINDOW_HEIGHT = 87,
        };

        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;

        typedef struct _PARTY_POSITION
        {
            float m_fPositionRcvd;
            float m_fPosition;
        } PARTY_POSITION;

    public:
        CNewUIDoppelGangerFrame();
        virtual ~CNewUIDoppelGangerFrame();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 1.2f

        void OpenningProcess();
        void ClosingProcess();

        void SetRemainTime(int iSeconds);
        void StopTimer(BOOL bFlag);

        void SetEnteredMonsters(int iCount) { m_iEnteredMonsters = iCount; }
        void SetMaxMonsters(int iCount) { m_iMaxMonsters = iCount; }
        void SetMonsterGauge(float fValue);
        void ResetPartyMemberInfo();
        void SetPartyMemberRcvd();
        void SetPartyMemberInfo(WORD wIndex, float fPosition);
        void SetIceWalkerMap(BOOL bEnable, float fPosition);

        void EnabledDoppelGangerEvent(BOOL bFlag) { m_bIsEnabled = bFlag; }
        BOOL IsDoppelGangerEnabled() { return m_bIsEnabled; }

    private:
        void LoadImages();
        void UnloadImages();

        int m_iEnteredMonsters;
        int m_iMaxMonsters;
        float m_fMonsterGauge;
        float m_fMonsterGaugeRcvd;

        std::map<WORD, PARTY_POSITION> m_PartyPositionMap;
        int m_iTime;
        BOOL m_bStopTimer;

        BOOL m_bIceWalkerEnabled;
        float m_fIceWalkerPositionRcvd;
        float m_fIceWalkerPosition;

        BOOL m_bIsEnabled;
    };
}
