// NewUIFriendWindow.h: interface for the CNewUIFriendWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIFRIENDWINDOW_H__22E51355_99DB_42E6_B735_39F2B20B7A1A__INCLUDED_)
#define AFX_NEWUIFRIENDWINDOW_H__22E51355_99DB_42E6_B735_39F2B20B7A1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIBase.h"
#include "UIWindows.h"

namespace SEASON3B
{
	class CNewUIManager;

	class CNewUIFriendWindow  : public CNewUIObj
	{
		CNewUIManager*	m_pNewUIMng;
		CUIWindowMgr*	m_pFriendWindowMgr;	// 친구쪽 UI 윈도우 관리자
		
	public:
		CNewUIFriendWindow();
		virtual ~CNewUIFriendWindow();

		bool Create(CNewUIManager*	pNewUIMng);
		void Release();

		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();		// 6.f
		
#ifdef KJH_FIX_WOPS_K29708_SHARE_LETTER
		void Reset();						// 친구창/창목록과 편지리스트 Reset
#endif // KJH_FIX_WOPS_K29708_SHARE_LETTER

		// 메세지 기능
		void SendUIMessage(int iMessage, int iParam1, int iParam2) { m_pFriendWindowMgr->SendUIMessage(iMessage, iParam1, iParam2); }
		void SendUIMessageToWindow(DWORD dwUIID, int iMessage, int iParam1, int iParam2) 
		{ m_pFriendWindowMgr->SendUIMessageToWindow(dwUIID, iMessage, iParam1, iParam2); }

		// 서버 기능
		void SetServerEnable(BOOL bFlag) { m_pFriendWindowMgr->SetServerEnable(bFlag); }
		BOOL IsServerEnable() { return m_pFriendWindowMgr->IsServerEnable(); }

#ifdef PSW_BUGFIX_FRIEND_LIST_CLEAR		
		void Reset() { m_pFriendWindowMgr->Reset(); }
#endif //PSW_BUGFIX_FRIEND_LIST_CLEAR

		// 윈도우 기능
		DWORD AddWindow(int iType, int x, int y, const char* strTitle, DWORD dwParentID = 0, int iOption = UIADDWND_NULL)
		{ return m_pFriendWindowMgr->AddWindow(iType, x, y, strTitle, dwParentID, iOption); }
		void AddWindowFinder(CUIBaseWindow * pWindow) { m_pFriendWindowMgr->AddWindowFinder(pWindow); }
		BOOL IsWindow(DWORD dwUIID) { return m_pFriendWindowMgr->IsWindow(dwUIID); }
		CUIBaseWindow * GetWindow(DWORD dwUIID) { return m_pFriendWindowMgr->GetWindow(dwUIID); }
		void SetWindowsEnable(DWORD bEnable) { m_pFriendWindowMgr->SetWindowsEnable(bEnable); }
		void HideAllWindow(BOOL bHide, BOOL bMainClose = FALSE) { m_pFriendWindowMgr->HideAllWindow(bHide, bMainClose); }
		void HideAllWindowClear() { m_pFriendWindowMgr->HideAllWindowClear(); }
		void OpenMainWnd(int x, int y) { m_pFriendWindowMgr->OpenMainWnd(x, y); }
		void CloseMainWnd() { m_pFriendWindowMgr->CloseMainWnd(); }
		DWORD GetTopWindowUIID() { return m_pFriendWindowMgr->GetTopWindowUIID(); } 
		DWORD GetTopNotMainWindowUIID() { return m_pFriendWindowMgr->GetTopNotMainWindowUIID(); }
		BOOL IsForceTopWindow(DWORD dwWindowUIID) { return m_pFriendWindowMgr->IsForceTopWindow(dwWindowUIID); }
		BOOL HaveForceTopWindow() { return m_pFriendWindowMgr->HaveForceTopWindow(); }
		void RefreshMainWndPalList() { m_pFriendWindowMgr->RefreshMainWndPalList(); }

		// 친구 기능
		CUIFriendWindow * GetFriendMainWindow() { return m_pFriendWindowMgr->GetFriendMainWindow(); }
		void SetAddFriendWindow(DWORD dwAddWindowUIID) { m_pFriendWindowMgr->SetAddFriendWindow(dwAddWindowUIID); }
		DWORD GetAddFriendWindow() { return m_pFriendWindowMgr->GetAddFriendWindow(); }

		// 채팅 기능
		void RefreshMainWndChatRoomList() { m_pFriendWindowMgr->RefreshMainWndChatRoomList(); }
		void SetChatReject(BOOL bChatReject) { m_pFriendWindowMgr->SetChatReject(bChatReject); }
		BOOL GetChatReject() { return m_pFriendWindowMgr->GetChatReject(); }

		// 편지 기능
		void RefreshMainWndLetterList() { m_pFriendWindowMgr->RefreshMainWndLetterList(); }
		BOOL LetterReadCheck(DWORD dwLetterID) { return m_pFriendWindowMgr->LetterReadCheck(dwLetterID); }
		void CloseLetterRead(DWORD dwLetterID) { m_pFriendWindowMgr->CloseLetterRead(dwLetterID); }
		void SetLetterReadWindow(DWORD dwLetterID, DWORD dwWindowUIID) { m_pFriendWindowMgr->SetLetterReadWindow(dwLetterID, dwWindowUIID); }
		DWORD GetLetterReadWindow(DWORD dwLetterID) { return m_pFriendWindowMgr->GetLetterReadWindow(dwLetterID); }

		BOOL IsRenderFrame() { return m_pFriendWindowMgr->IsRenderFrame(); }

		static CFriendList* GetFriendList();	// 친구 리스트
		static CLetterList* GetLetterList();	// 편지 리스트
		static CUIFriendMenu* GetFriendMenu();	// 친구 메뉴
	};

}

#define g_pFriendList SEASON3B::CNewUIFriendWindow::GetFriendList()
#define g_pLetterList SEASON3B::CNewUIFriendWindow::GetLetterList()
#define g_pFriendMenu SEASON3B::CNewUIFriendWindow::GetFriendMenu()


#endif // !defined(AFX_NEWUIFRIENDWINDOW_H__22E51355_99DB_42E6_B735_39F2B20B7A1A__INCLUDED_)
