// NewUIDoppelGangerFrame.h: interface for the CNewUIDoppelGangerFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIDOPPELGANGERFRAME_H__91BBA47F_0EE6_4FA3_A600_8004EC875FD6__INCLUDED_)
#define AFX_NEWUIDOPPELGANGERFRAME_H__91BBA47F_0EE6_4FA3_A600_8004EC875FD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef YDG_ADD_DOPPELGANGER_UI

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

		CNewUIManager*				m_pNewUIMng;
		POINT						m_Pos;
		
		typedef struct _PARTY_POSITION
		{
			float m_fPositionRcvd;
			float m_fPosition;		// 출력용(보간)
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
		float m_fMonsterGauge;		// 출력용(보간)
		float m_fMonsterGaugeRcvd;

		std::map<WORD, PARTY_POSITION> m_PartyPositionMap;
		int m_iTime;
		BOOL m_bStopTimer;

		// 아이스워커 포지션
		BOOL m_bIceWalkerEnabled;
		float m_fIceWalkerPositionRcvd;
		float m_fIceWalkerPosition;		// 출력용(보간)

		BOOL m_bIsEnabled;
	};
}
#endif	// YDG_ADD_DOPPELGANGER_UI

#endif // !defined(AFX_NEWUIDOPPELGANGERFRAME_H__91BBA47F_0EE6_4FA3_A600_8004EC875FD6__INCLUDED_)
