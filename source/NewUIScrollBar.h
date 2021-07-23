//=============================================================================
//
//	NewUIScrollBar.h
//
//	Copyright 2010 Webzen Mu-Studio
//
//=============================================================================
#ifndef _NEWUISCROLLBAR_H_
#define _NEWUISCROLLBAR_H_

#ifdef KJW_ADD_NEWUI_SCROLLBAR

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIChatLogWindow.h"

namespace SEASON3B
{
	class CNewUIScrollBar : public CNewUIObj
	{
		//===================================================================
		// ENUM IMAGE_LIST TYPE
		//===================================================================
		enum KEnumTypeIMAGE_LIST										// 스크롤바 텍스쳐 관련 enum
		{
			IMAGE_SCROLL_TOP	= CNewUIChatLogWindow::IMAGE_SCROLL_TOP,			// newui_scrollbar_up.tga (7,3)
			IMAGE_SCROLL_MIDDLE	= CNewUIChatLogWindow::IMAGE_SCROLL_MIDDLE,			// newui_scrollbar_m.tga (7,15)
			IMAGE_SCROLL_BOTTOM	= CNewUIChatLogWindow::IMAGE_SCROLL_BOTTOM,			// newui_scrollbar_down.tga (7,3)
			IMAGE_SCROLLBAR_ON	= CNewUIChatLogWindow::IMAGE_SCROLLBAR_ON,			// newui_scroll_On.tga (15,30)	
			IMAGE_SCROLLBAR_OFF	= CNewUIChatLogWindow::IMAGE_SCROLLBAR_OFF,			// newui_scroll_Off.tga (15,30)
		};
		
		//===================================================================
		// ENUM SCROLLBTN TYPE
		//===================================================================
		enum KEnumTypeSCROLLBTN											// 스크롤바 크기 관련 enum
		{
			SCROLLBTN_WIDTH = 15,
			SCROLLBTN_HEIGHT = 30,
			SCROLLBAR_TOP_WIDTH = 7,
			SCROLLBAR_TOP_HEIGHT = 3,
			SCROLLBAR_MIDDLE_WIDTH = 7,
			SCROLLBAR_MIDDLE_HEIGHT = 15
		};

		//===================================================================
		// ENUM SCROLLBAR_MOUSEBTN TYPE
		//===================================================================
		enum KEnumTypeSCROLLBAR_MOUSEBTN								// 스크롤바 마우스 이벤트 관련 enum
		{
			SCROLLBAR_MOUSEBTN_INVALID	 = -1,
			SCROLLBAR_MOUSEBTN_FIRST,
			SCROLLBAR_MOUSEBTN_NORMAL = SCROLLBAR_MOUSEBTN_FIRST,		// 스크롤바 보통상태
			SCROLLBAR_MOUSEBTN_OVER,									// 스크롤바 버튼 위에 마우스가 놓인 상태
			SCROLLBAR_MOUSEBTN_CLICKED,									// 스크롤바 버튼 클릭 상태
			SCROLLBAR_MOUSEBTN_TOTAL,						
			SCROLLBAR_MOUSEBTN_LAST		= SCROLLBAR_MOUSEBTN_TOTAL - 1,
		};

	protected:
		int							m_iHeight;							// 스크롤바의 높이
		POINT						m_ptPos;							// 스크롤바의 위치
		
		POINT						m_ptScrollBtnStartPos;				// 스크롤바 버튼의 시작 위치
		POINT						m_ptScrollBtnPos;					// 스크롤바 버튼의 위치
		
		int							m_iScrollBarPickGap;				// 스크롤바의 클릭 지점의 높이 차이
		
		int							m_iScrollBarMovePixel;				
		int							m_iScrollBarHeightPixel;			// 스크롤바의 높이 픽셀 수
		int							m_iScrollBarMiddleNum;				// 스크롤바의 중간프레임의 수
		int							m_iScrollBarMiddleRemainderPixel;	// 스크롤바의 중간 프레임을 그리고 남는 픽셀 수
		
		int							m_iScrollBtnMouseEvent;				// 스크롤 버튼 마우스 클릭 이벤트
		bool						m_bScrollBtnActive;					// 스크롤버튼 활성화
		
		float						m_fPercentOfSize;					// 스크롤바 버튼위 위치( 0.0f ~ 1.0f )
		
		int							m_iBeginPos;						// 시작지점
		int							m_iCurPos;							// 현재 위치
		int							m_iMaxPos;							// 최대 이동가능한 위치

	public:
		CNewUIScrollBar();
		virtual ~CNewUIScrollBar();
		
		bool Create( int iX, int iY, int iHeight );					
		void Release();
		
		float GetLayerDepth();
		
		void LoadImages();
		void UnloadImages();
		
		void SetPos(int x, int y);
		
		bool UpdateBtnEvent();
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		
		bool Update();
		bool Render();
		
		void UpdateScrolling();
		void ScrollUp( int iMoveValue );
		void ScrollDown( int iMoveValue );
		
		float GetPercent() { return m_fPercentOfSize; }
		void SetPercent( float fPercent );
		
		int GetMaxPos() { return m_iMaxPos; }
		void SetMaxPos( int iMaxPos );
		
		int GetCurPos() { return m_iCurPos; }
		void SetCurPos( int iMoveValue );
	};
}

#endif // KJW_ADD_NEWUI_SCROLLBAR

#endif // _NEWUISCROLLBAR_H_