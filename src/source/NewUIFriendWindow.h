// NewUIFriendWindow.h: interface for the CNewUIFriendWindow class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "NewUIBase.h"
#include "UIWindows.h"

namespace SEASON3B
{
    class CNewUIManager;

    class CNewUIFriendWindow : public CNewUIObj
    {
        CNewUIManager* m_pNewUIMng;
        CUIWindowMgr* m_pFriendWindowMgr;

    public:
        CNewUIFriendWindow();
        virtual ~CNewUIFriendWindow();

        bool Create(CNewUIManager* pNewUIMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();		// 6.f

        void Reset();

        void SendUIMessage(int iMessage, int iParam1, int iParam2) { m_pFriendWindowMgr->SendUIMessage(iMessage, iParam1, iParam2); }
        void SendUIMessageToWindow(DWORD dwUIID, int iMessage, int iParam1, int iParam2)
        {
            m_pFriendWindowMgr->SendUIMessageToWindow(dwUIID, iMessage, iParam1, iParam2);
        }

        void SetServerEnable(BOOL bFlag) { m_pFriendWindowMgr->SetServerEnable(bFlag); }
        BOOL IsServerEnable() { return m_pFriendWindowMgr->IsServerEnable(); }

        DWORD AddWindow(int iType, int x, int y, const wchar_t* strTitle, DWORD dwParentID = 0, int iOption = UIADDWND_NULL)
        {
            return m_pFriendWindowMgr->AddWindow(iType, x, y, strTitle, dwParentID, iOption);
        }
        void AddWindowFinder(CUIBaseWindow* pWindow) { m_pFriendWindowMgr->AddWindowFinder(pWindow); }
        BOOL IsWindow(DWORD dwUIID) { return m_pFriendWindowMgr->IsWindow(dwUIID); }
        CUIBaseWindow* GetWindow(DWORD dwUIID) { return m_pFriendWindowMgr->GetWindow(dwUIID); }
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

        CUIFriendWindow* GetFriendMainWindow() { return m_pFriendWindowMgr->GetFriendMainWindow(); }
        void SetAddFriendWindow(DWORD dwAddWindowUIID) { m_pFriendWindowMgr->SetAddFriendWindow(dwAddWindowUIID); }
        DWORD GetAddFriendWindow() { return m_pFriendWindowMgr->GetAddFriendWindow(); }

        void RefreshMainWndChatRoomList() { m_pFriendWindowMgr->RefreshMainWndChatRoomList(); }
        void SetChatReject(BOOL bChatReject) { m_pFriendWindowMgr->SetChatReject(bChatReject); }
        BOOL GetChatReject() { return m_pFriendWindowMgr->GetChatReject(); }

        void RefreshMainWndLetterList() { m_pFriendWindowMgr->RefreshMainWndLetterList(); }
        BOOL LetterReadCheck(DWORD dwLetterID) { return m_pFriendWindowMgr->LetterReadCheck(dwLetterID); }
        void CloseLetterRead(DWORD dwLetterID) { m_pFriendWindowMgr->CloseLetterRead(dwLetterID); }
        void SetLetterReadWindow(DWORD dwLetterID, DWORD dwWindowUIID) { m_pFriendWindowMgr->SetLetterReadWindow(dwLetterID, dwWindowUIID); }
        DWORD GetLetterReadWindow(DWORD dwLetterID) { return m_pFriendWindowMgr->GetLetterReadWindow(dwLetterID); }

        BOOL IsRenderFrame() { return m_pFriendWindowMgr->IsRenderFrame(); }

        static CFriendList* GetFriendList();
        static CLetterList* GetLetterList();
        static CUIFriendMenu* GetFriendMenu();
    };
}

#define g_pFriendList SEASON3B::CNewUIFriendWindow::GetFriendList()
#define g_pLetterList SEASON3B::CNewUIFriendWindow::GetLetterList()
#define g_pFriendMenu SEASON3B::CNewUIFriendWindow::GetFriendMenu()
