// CNewUIEmpireGuardianTimer.h: interface for the CNewUIEmpireGuardianTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CNewUIEmpireGuardianTimer_H__8AD8FE50_F906_4AB7_8944_FD34CB3D3F91__INCLUDED_)
#define AFX_CNewUIEmpireGuardianTimer_H__8AD8FE50_F906_4AB7_8944_FD34CB3D3F91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef LDK_ADD_EMPIREGUARDIAN_UI

#include "NewUIBase.h"
#include "NewUIManager.h"

namespace SEASON3B
{
	class CNewUIEmpireGuardianTimer : public CNewUIObj  
	{
	public:	
		enum EG_DAY_MAP_LIST
		{
			EG_MONDAY,
			EG_TUESDAY,
			EG_WEDNESDAY,
			EG_THURSDAY,
			EG_FRIDAY,
			EG_SATURDAY,
			EG_SUNDAY,
		};

		enum IMAGE_LIST
		{		
			IMAGE_EMPIREGUARDIAN_TIMER_WINDOW = BITMAP_EMPIREGUARDIAN_TIMER_BEGIN,
		};
		
	private:
		enum EMPIREGUARDIAN_TIME_WINDOW_SIZE
		{
			TIMER_WINDOW_WIDTH = 124,
			TIMER_WINDOW_HEIGHT = 81,
		};
		
		CNewUIManager*				m_pNewUIMng;
		POINT						m_Pos;
		
	public:
		CNewUIEmpireGuardianTimer();
		virtual ~CNewUIEmpireGuardianTimer();
		
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
		
		void SetType(int iType) { m_iType = iType; }
		void SetRemainTime(DWORD tick) { m_dTime = tick; }
		void SetDay(int iDay)	{ m_iDay = iDay; }
		void SetZone(int iZone)	{ m_iZone = iZone; }
		void SetMonsterCount(int iMonsterCount)	{ m_iMonsterCount = iMonsterCount; }
		
		const int GetDay()	{ return m_iDay; }
		const int GetZone()	{ return m_iZone; }
				
	private:
		void LoadImages();
		void UnloadImages();
		
		int m_iType;
		DWORD m_dTime;
		int m_iDay;
		int m_iZone;
		int m_iMonsterCount;
	};
}
#endif	// LDK_ADD_EMPIREGUARDIAN_UI

#endif // !defined(AFX_CNewUIEmpireGuardianTimer_H__8AD8FE50_F906_4AB7_8944_FD34CB3D3F91__INCLUDED_)

