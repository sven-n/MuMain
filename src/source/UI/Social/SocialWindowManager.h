#pragma once

// The friend/mail/chat family's own window manager: it owns the windows, arranges them, and runs
// the UI-message queue they talk over. CFriendWindow (FriendWindow.h) is the single seam between
// it and mu::ui::window::CManager.

#include "UI/Social/SocialWindowBase.h"
#include "UI/Social/ChatRoomWindow.h"
#include "UI/Social/FriendShellWindow.h"
#include "UI/Social/LetterReadWindow.h"
#include "UI/Social/LetterWriteWindow.h"
#include "Network/Server/WSclient.h"
#include "UI/Social/SocialUpdates.h"
#include <list>
#include <map>
#include <mutex>

const int UIWND_DEFAULT = -1;

enum UIWINDOWSTYPE

{
    // 0 was UIWNDTYPE_EMPTY, whose window class nothing ever asked for; the rest keep their values.
    UIWNDTYPE_CHAT = 1,
    UIWNDTYPE_CHAT_READY,
    UIWNDTYPE_FRIENDMAIN,
    UIWNDTYPE_READLETTER,
    UIWNDTYPE_WRITELETTER,
};

enum UIADDWINDOWOPTION

{
    UIADDWND_NULL = 0,
    UIADDWND_FORCEPOSITION = 1
};

class FriendWindowViews;

class CFriendList
{
public:
    // cppcheck-suppress uninitMemberVar
    CFriendList() : m_iCurrentSortType(0) {}
    ~CFriendList()
    {
        ClearFriendList();
    }
    void AddFriend(const wchar_t* pszID, BYTE Number, BYTE Server);
    void RemoveFriend(const wchar_t* pszID);
    void ClearFriendList();
    int UpdateFriendList(std::deque<GUILDLIST_TEXT>& pDestData, const wchar_t* pszID);
    void UpdateFriendState(const wchar_t* pszID, BYTE Number, BYTE Server);
    void UpdateAllFriendState(BYTE Number, BYTE Server);
    void Sort(int iType = -1);
    int GetCurrentSortType()
    {
        return m_iCurrentSortType;
    }

private:
    int m_iCurrentSortType;
    std::deque<GUILDLIST_TEXT> m_FriendList;
    std::deque<GUILDLIST_TEXT>::iterator m_FriendListIter;
};

class CLetterList
{
public:
    // cppcheck-suppress uninitMemberVar
    CLetterList() : m_iCurrentSortType(0) {}
    ~CLetterList()
    {
        ClearLetterList();
    }
    void AddLetter(DWORD dwLetterID, const wchar_t* pszID, const wchar_t* pszText, const wchar_t* pszDate,
                   const wchar_t* pszTime, BOOL bIsRead);
    void RemoveLetter(DWORD dwLetterID);
    void ClearLetterList();
    int UpdateLetterList(std::deque<LETTERLIST_TEXT>& pDestData, DWORD dwSelectLineNum);
    void Sort(int iType = -1);
    DWORD GetPrevLetterID(DWORD dwLetterID);
    DWORD GetNextLetterID(DWORD dwLetterID);
    LETTERLIST_TEXT* GetLetter(DWORD dwLetterID);
    void ResetLetterSelect(BOOL bFlag);
    BOOL CheckNoReadLetter();
    int GetCurrentSortType()
    {
        return m_iCurrentSortType;
    }

    void CacheLetterText(DWORD dwIndex, const UI::Social::LetterBody& body);
    const UI::Social::LetterBody* GetLetterText(DWORD dwIndex);
    void RemoveLetterTextCache(DWORD dwIndex);
    void ClearLetterTextCache();

    int GetLineNum(DWORD dwLetterID);
    int GetLetterCount()
    {
        return m_LetterList.size();
    }

private:
    int m_iCurrentSortType;
    std::deque<LETTERLIST_TEXT> m_LetterList;
    std::deque<LETTERLIST_TEXT>::iterator m_LetterListIter;

    std::map<DWORD, UI::Social::LetterBody, std::less<DWORD>> m_LetterCache;
    std::map<DWORD, UI::Social::LetterBody, std::less<DWORD>>::iterator m_LetterCacheIter;
};

typedef std::map<DWORD, CUIBaseWindow*, std::less<DWORD>> WndMap;

class CUIWindowMgr : public CUIMessage
{
public:
    CUIWindowMgr();
    virtual ~CUIWindowMgr();

    void Reset();
    DWORD AddWindow(int iWindowType, int iPos_x, int iPos_y, const wchar_t* pszTitle, DWORD dwParentID = 0,
                    int iOption = UIADDWND_NULL);
    void RemoveWindow(DWORD dwUIID);
    void Render();
    // Shows the family's documents in the draw order Render() uses; hides them all when
    // !familyShown.
    void SyncRmlViews(bool familyShown);
    // True while an RmlUi input of the window holds the keyboard, so selecting it leaves the caret
    // where it is.
    bool RmlFieldHasFocus(DWORD dwUIID) const;
    void DoAction();
    void ShowHideWindow(DWORD dwUIID, BOOL bShowWindow);
    void HideAllWindow(BOOL bHide, BOOL bMainClose = FALSE);
    void HideAllWindowClear();
    CUIBaseWindow* GetWindow(DWORD dwUIID);
    // The window of this family whose own RmlUi field holds the keyboard, or nullptr.
    CUIBaseWindow* GetFieldFocusWindow() const;
    BOOL IsWindow(DWORD dwUIID);
    CUIFriendWindow* GetFriendMainWindow()
    {
        // cppcheck-suppress dangerousTypeCast
        return (CUIFriendWindow*)GetWindow(m_dwMainWindowUIID);
    }
    void SetWindowsEnable(DWORD bWindowsEnable)
    {
        m_bWindowsEnable = bWindowsEnable;
    }
    BOOL GetWindowsEnable()
    {
        return m_bWindowsEnable;
    }
    DWORD GetTopWindowUIID()
    {
        return (m_WindowArrangeList.empty() == TRUE ? 0 : *m_WindowArrangeList.rbegin());
    }
    DWORD GetTopNotMainWindowUIID();

    void SendUIMessageToWindow(DWORD dwUIID, int iMessage, LONG_PTR iParam1, LONG_PTR iParam2);

    void OpenMainWnd(int iPos_x, int iPos_y);
    void CloseMainWnd();
    void RefreshMainWndPalList()
    {
        if (m_dwMainWindowUIID != 0)
            GetFriendMainWindow()->RefreshPalList();
    }
    void RefreshMainWndLetterList()
    {
        if (m_dwMainWindowUIID != 0)
            GetFriendMainWindow()->RefreshLetterList();
    }
    void RefreshMainWndChatRoomList();

    void SetChatReject(BOOL bChatReject)
    {
        m_bChatReject = bChatReject;
    }
    BOOL GetChatReject()
    {
        return m_bChatReject;
    }

    BOOL LetterReadCheck(DWORD dwLetterID);
    void CloseLetterRead(DWORD dwLetterID);
    void SetLetterReadWindow(DWORD dwLetterID, DWORD dwWindowUIID);
    DWORD GetLetterReadWindow(DWORD dwLetterID);

    void SetServerEnable(BOOL bFlag);
    BOOL IsServerEnable()
    {
        return m_bServerEnable;
    }

    void AddForceTopWindowList(DWORD dwWindowUIID);
    void RemoveForceTopWindowList(DWORD dwWindowUIID);
    BOOL IsForceTopWindow(DWORD dwWindowUIID);
    BOOL HaveForceTopWindow()
    {
        return !m_ForceTopWindowList.empty();
    }

    BOOL IsRenderFrame()
    {
        return m_bRenderFrame;
    }

protected:
    void HandleMessage();

public:
    BOOL m_bRenderFrame;

protected:
    std::unique_ptr<FriendWindowViews> m_pRmlViews;
    BOOL m_bWindowsEnable;
    DWORD m_dwMainWindowUIID;
    WndMap m_WindowMap;
    WndMap::iterator m_WindowMapIter;
    std::list<DWORD> m_WindowArrangeList;
    std::list<DWORD>::iterator m_WindowArrangeListIter;
    std::list<DWORD>::reverse_iterator m_WindowReverseArrangeListIter;
    std::map<DWORD, DWORD, std::less<DWORD>> m_LetterReadMap;
    std::map<DWORD, DWORD, std::less<DWORD>>::iterator m_LetterReadMapIter;
    BOOL m_bCurrentHideWindowState;
    std::list<DWORD> m_HideWindowList;
    std::list<DWORD> m_ForceTopWindowList;

    int m_iMainWindowPos_x, m_iMainWindowPos_y;
    int m_iMainWindowWidth, m_iMainWindowHeight;
    int m_iMainWindowBackPos_y, m_iMainWindowBackHeight;
    BOOL m_bIsMainWindowMaximize;
    BOOL m_bChatReject;
    int m_iLastFriendWindowTabIndex;

    BOOL m_bServerEnable;
    int m_iFriendMainWindowTitleNumber;
};

class CUIFriendMenu : public CUIBaseWindow
{
public:
    CUIFriendMenu()
    {
        Init();
    }
    virtual ~CUIFriendMenu()
    {
        Reset();
    }

    void Reset();
    void Init();
    void AddWindow(DWORD dwUIID, CUIBaseWindow* pWindow);
    void RemoveWindow(DWORD dwUIID);

    void ShowMenu(BOOL bHotKey = FALSE);
    void HideMenu();

    void SetNewChatAlert(DWORD dwAlertWindowID);
    void SetNewChatAlertOff(DWORD dwAlertWindowID);
    BOOL IsNewChatAlert();
    void SetNewMailAlert(BOOL bAlert);
    BOOL IsNewMailAlert()
    {
        return m_bNewMailAlert;
    }

    int GetBlinkTemp();
    void IncreaseBlinkTemp();
    int GetLetterBlink();
    void IncreaseLetterBlink();

    DWORD CheckChatRoomDuplication(const wchar_t* pszTargetName);
    void SendChatRoomConnectCheck();
    void UpdateAllChatWindowInviteList();

    BOOL IsHotkeyEnable()
    {
        return m_bHotKey;
    }

    void AddRequestWindow(const wchar_t* szTargetName);
    BOOL IsRequestWindow(const wchar_t* szTargetName);
    void RemoveRequestWindow(const wchar_t* szTargetName);
    void RemoveAllRequestWindow();

    void CloseAllChatWindow();
    void LockAllChatWindow();

protected:
    virtual BOOL HandleMessage();

protected:
    std::deque<DWORD> m_WindowList;
    std::deque<DWORD>::iterator m_WindowListIter;
    std::deque<DWORD>::iterator m_WindowListSelectIter;
    float m_fLineHeight;
    int m_iFriendMenuPos_y;
    int m_iFriendMenuHeight;
    float m_fMenuAlpha;
    float m_fMenuAlphaAdd;
    std::deque<DWORD> m_NewChatWindowList;
    BOOL m_bNewMailAlert;
    int m_iBlinkTemp;
    int m_iLetterBlink;
    BOOL m_bHotKey;
    std::deque<wchar_t*> m_RequestChatWindowList;
    std::deque<wchar_t*>::iterator m_RequestChatWindowListIter;
};
