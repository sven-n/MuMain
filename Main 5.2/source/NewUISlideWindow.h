// NewUISlideWindow.h: interface for the CNewUISlideWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUISLIDEWINDOW_H__9A5B5D3E_1F9A_4778_8DC7_5367287F45AC__INCLUDED_)
#define AFX_NEWUISLIDEWINDOW_H__9A5B5D3E_1F9A_4778_8DC7_5367287F45AC__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "UIControls.h"

namespace SEASON3B
{
	class CNewUIManager;
	
	class CNewUISlideWindow  :	public CNewUIObj
	{
		CNewUIManager*	m_pNewUIMng;
	public:
		CNewUISlideWindow();
		virtual ~CNewUISlideWindow();

		bool Create(CNewUIManager* pNewUIMng);
		void Release();

		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();		// 1.91f

		// wrapping
		void Init() { m_pSlideMgr->Init(); }
		void CreateSlideText() { m_pSlideMgr->CreateSlideText(); }
		void AddSlide(int iLoopCount, int iLoopDelay, const char* strText, int iType, float fSpeed, DWORD dwTextColor = (255<<24)+(200<<16)+(220<<8)+(230))
		{ 
			m_pSlideMgr->AddSlide(iLoopCount, iLoopDelay, strText, iType, fSpeed, dwTextColor);
		}

	private:
		CSlideHelpMgr* m_pSlideMgr;
	};

}

extern std::string g_strSelectedML;

#endif // !defined(AFX_NEWUISLIDEWINDOW_H__9A5B5D3E_1F9A_4778_8DC7_5367287F45AC__INCLUDED_)
