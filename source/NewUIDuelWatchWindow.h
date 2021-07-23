// NewUIDuelWatchWindow.h: interface for the CNewUIDuelWatchWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIDUELWATCHWINDOW_H__3E1DAC48_FC3E_463D_8DCE_5CC18F4DFC22__INCLUDED_)
#define AFX_NEWUIDUELWATCHWINDOW_H__3E1DAC48_FC3E_463D_8DCE_5CC18F4DFC22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef YDG_ADD_NEW_DUEL_UI

#include "NewUIBase.h"
#include "NewUIButton.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIMyQuestInfoWindow.h"

namespace SEASON3B
{
	class CNewUIDuelWatchWindow : public CNewUIObj  
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_DUELWATCHWINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
			IMAGE_DUELWATCHWINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,	
			IMAGE_DUELWATCHWINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
			IMAGE_DUELWATCHWINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
			IMAGE_DUELWATCHWINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
			IMAGE_DUELWATCHWINDOW_BUTTON = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
			IMAGE_DUELWATCHWINDOW_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
		};
	private:
		enum
		{
			INVENTORY_WIDTH = 190,
			INVENTORY_HEIGHT = 429,
		};

		CNewUIManager*			m_pNewUIMng;
		POINT m_Pos;

		CNewUIButton m_BtnChannel[4];		// 관전 버튼
		BOOL m_bChannelEnable[4];	// 방 입장 가능 상태인가(개설/비개설)
	public:
		CNewUIDuelWatchWindow();
		virtual ~CNewUIDuelWatchWindow();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		void OpeningProcess();
		void ClosingProcess();

		float GetLayerDepth();	//. 5.0f

	private:
		void LoadImages();
		void UnloadImages();

		void RenderFrame();
		bool BtnProcess();

		void InitButton(CNewUIButton * pNewUIButton, int iPos_x, int iPos_y, const unicode::t_char * pCaption);
	};
}
#endif	// YDG_ADD_NEW_DUEL_UI

#endif // !defined(AFX_NEWUIDUELWATCHWINDOW_H__3E1DAC48_FC3E_463D_8DCE_5CC18F4DFC22__INCLUDED_)
