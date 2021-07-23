// NewUICommandWindow.h: interface for the CNewUICommandWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_)
#define AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#ifdef ADD_PCROOM_POINT_SYSTEM
#include "PCRoomPoint.h"
#endif	// ADD_PCROOM_POINT_SYSTEM

#ifdef KJH_DEL_PC_ROOM_SYSTEM
#define MAX_COMMAND_BTN		( 11 )
#else // KJH_DEL_PC_ROOM_SYSTEM
#ifdef ADD_PCROOM_POINT_SYSTEM
	#define MAX_COMMAND_BTN		( 12 )
#else // ADD_PCROOM_POINT_SYSTEM
	#define MAX_COMMAND_BTN		( 11 )
#endif // ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM

#define COMMAND_BTN_INTERVAL_SIZE	(1)				// 커맨드 버튼들 사이의 간격

namespace SEASON3B
{
	class CNewUICommandWindow : public CNewUIObj 
	{
	public:
		enum eIMAGE_LIST
		{		
			// Base Window (Reference)
			IMAGE_COMMAND_BASE_WINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
			IMAGE_COMMAND_BASE_WINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,			//. newui_item_back01.tga	(190,64)
			IMAGE_COMMAND_BASE_WINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,			//. newui_item_back02-l.tga	(21,320)
			IMAGE_COMMAND_BASE_WINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
			IMAGE_COMMAND_BASE_WINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,		//. newui_item_back03.tga	(190,45)
			IMAGE_COMMAND_BASE_WINDOW_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,		//. newui_exit_00.tga
			IMAGE_COMMAND_BTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,						//. newui_btn_empty.tga
			IMAGE_COMMAND_SELECTID_BG = BITMAP_COMMAND_WINDOW_BEGIN,								//. newUI_cursorid_wnd.jpg	(128,32)
		};

		enum eCOMMAND_WINDOW_SIZE
		{
			COMMAND_WINDOW_WIDTH = 190,
			COMMAND_WINDOW_HEIGHT = 429,
		};

	private:
		CNewUIManager*				m_pNewUIMng;
		POINT						m_Pos;

		// RadioGroupButton
		CNewUIButton				m_BtnCommand[MAX_COMMAND_BTN];
		// Exit Button
		CNewUIButton				m_BtnExit;
		int							m_iCurSelectCommand;
		int							m_iCurMouseCursor;
#ifdef BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
		bool						m_bSelectedChar;
		bool						m_bCanCommand;
#endif // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
	
		
	public:		
		CNewUICommandWindow();
		virtual ~CNewUICommandWindow();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		bool BtnProcess();
		
		float GetLayerDepth();	//. 4.6f
		
		void OpenningProcess();
		void ClosingProcess();
		
		int	GetCurCommandType( );		
		
		void SetMouseCursor( int iCursorType );
		int	 GetMouseCursor( );
		
		bool CommandTrade(CHARACTER* pSelectedCha);				// 거래 명령
		bool CommandPurchase(CHARACTER* pSelectedCha);			// 구매 명령
		bool CommandParty(SHORT iChaKey);						// 파티 명령
		bool CommandWhisper(CHARACTER* pSelectedCha);			// 귓속말 명령	
		bool CommandGuild(CHARACTER* pSelectedCha);				// 길드가입 명령
 		bool CommandGuildUnion(CHARACTER* pSelectedCha);		// 길드연합 명령
 		bool CommandGuildRival(CHARACTER* pSelectedCha);		// 적대길드 명령
 		bool CommandCancelGuildRival(CHARACTER* pSelectedCha);	// 적대길드해제 명령
 		bool CommandAddFriend(CHARACTER* pSelectedCha);			// 친구추가 명령
 		bool CommandFollow(int iSelectedChaIndex);			// 따라가기 명령
 		int CommandDual(CHARACTER* pSelectedCha);						// 결투명령
#ifndef KJH_DEL_PC_ROOM_SYSTEM				// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM	
 		bool CommandPCBangPoint();				// PC방 포인트 상점 명령
#endif // ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM

	private:
		void LoadImages( );
		void UnloadImages( );
		void InitButtons( );
		void RenderBaseWindow( );
		void RunCommand( );
		void SelectCommand( );
		void SetBtnState(int iBtnType, bool bStateDown);

	};
};

#endif // !defined(AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_)
