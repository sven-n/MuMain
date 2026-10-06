#pragma once

// The base class of every window in the friend/mail/chat family, and the messages and states its
// manager runs them through. The windows themselves are RmlUi documents; what this carries is the
// bookkeeping CUIWindowMgr needs -- identity, parent, state, geometry and the message queue.

#include "Engine/Object/ZzzInfomation.h"
#include "Network/Server/WSclient.h"
#include "GameLogic/Quests/QuestMng.h"
#include "Core/Time/Timer.h"
#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace UI::Social { class FriendShell; class ChatRoomView; class LetterReadView; class LetterWriteView; }

enum UISTATES
{
    UISTATE_NORMAL = 0,
    UISTATE_RESIZE,
    UISTATE_SCROLL,
    UISTATE_HIDE,
    UISTATE_MOVE,
    UISTATE_READY,
    UISTATE_DISABLE
};

typedef struct
{
    BOOL m_bIsSelected;
    wchar_t m_szID[MAX_USERNAME_SIZE + 1];
    BYTE m_Number;
    BYTE m_Server;
    BYTE m_GuildStatus;
} GUILDLIST_TEXT;

typedef struct
{
    BOOL m_bIsSelected;
    DWORD m_dwLetterID;
    wchar_t m_szID[MAX_USERNAME_SIZE + 1];
    wchar_t m_szText[MAX_TEXT_LENGTH + 1];
    wchar_t m_szDate[16];
    wchar_t m_szTime[16];
    BOOL m_bIsRead;
} LETTERLIST_TEXT;

enum UI_MESSAGE_ENUM
{
    UI_MESSAGE_NULL = 0,
    UI_MESSAGE_SELECT,
    UI_MESSAGE_HIDE,
    UI_MESSAGE_MAXIMIZE,
    UI_MESSAGE_CLOSE,
    UI_MESSAGE_BOTTOM,
    UI_MESSAGE_SELECTED,
    UI_MESSAGE_TEXTINPUT,
    UI_MESSAGE_BTNLCLICK,
    UI_MESSAGE_TXTRETURN,
    UI_MESSAGE_YNRETURN,
    UI_MESSAGE_LISTDBLCLICK,
    UI_MESSAGE_LISTSCRLTOP,
    UI_MESSAGE_LISTSELUP,
    UI_MESSAGE_LISTSELDOWN
};

struct UI_MESSAGE
{
    int m_iMessage;
    LONG_PTR m_iParam1;
    LONG_PTR m_iParam2;
};

class CUIMessage
{
public:
    CUIMessage() {}
    virtual ~CUIMessage()
    {
        m_MessageList.clear();
    }

    void SendUIMessage(int iMessage, LONG_PTR iParam1, LONG_PTR iParam2);
    void GetUIMessage();

protected:
    std::deque<UI_MESSAGE> m_MessageList;
    UI_MESSAGE m_WorkMessage;
};


extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;

class CUIBaseWindow : public CUIMessage
{
public:
    CUIBaseWindow();
    virtual ~CUIBaseWindow();

    DWORD GetUIID() const
    {
        return m_dwUIID;
    }
    void SetParentUIID(DWORD dwParentUIID)
    {
        m_dwParentUIID = dwParentUIID;
    }
    DWORD GetParentUIID() const
    {
        return m_dwParentUIID;
    }
    void SetState(int iState)
    {
        m_iState = iState;
    }
    int GetState() const
    {
        return m_iState;
    }
    void SetPosition(int iPos_x, int iPos_y)
    {
        m_iPos_x = iPos_x;
        m_iPos_y = iPos_y;
    }
    int GetPosition_x() const
    {
        return m_iPos_x;
    }
    int GetPosition_y() const
    {
        return m_iPos_y;
    }
    void SetSize(int iWidth, int iHeight)
    {
        m_iWidth = iWidth;
        m_iHeight = iHeight;
    }
    int GetWidth() const
    {
        return m_iWidth;
    }
    int GetHeight() const
    {
        return m_iHeight;
    }

    // Queues a message and handles it now, as if the window's own frame had run.
    void SendUIMessageDirect(int iMessage, int iParam1, int iParam2);
    // Once a frame from CUIWindowMgr::DoAction(): its queued messages, then the window's own work.
    virtual BOOL DoAction(BOOL bMessageOnly = FALSE);

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);

    virtual void Refresh() {}
    virtual void SetTitle(const wchar_t* pszTitle);
    void SetReturnText(const wchar_t* text);
    std::wstring TakeReturnText();
    const wchar_t* GetTitle()
    {
        return m_strTitle.c_str();
    }
    // The window's own view toggles it (UI_MESSAGE_MAXIMIZE).
    virtual void Maximize() {}

    BOOL HaveTextBox()
    {
        return m_bHaveTextBox;
    }
    void GetBackPosition(BOOL* pbIsMaximize, int* piBackPos_y, int* piBackHeight)
    {
        *pbIsMaximize = m_bIsMaximize;
        *piBackPos_y = m_iBackPos_y;
        *piBackHeight = m_iBackHeight;
    }
    void SetBackPosition(BOOL bIsMaximize, int iBackPos_y, int iBackHeight)
    {
        m_bIsMaximize = bIsMaximize;
        m_iBackPos_y = iBackPos_y;
        m_iBackHeight = iBackHeight;
    }

    virtual BOOL CloseCheck()
    {
        return TRUE;
    }

    // A window of this family that owns an RmlUi document of its own. FriendWindowViews::Sync()
    // drives these; nothing else of the family is drawn by the manager any more.
    virtual bool HasSemanticView() const
    {
        return false;
    }
    // True while one of this window's own RmlUi fields holds the keyboard, so selecting it does
    // not steal the caret -- what the native CUITextInputBox's focus used to say.
    virtual bool SemanticFieldHasFocus() const
    {
        return false;
    }
    virtual bool SyncSemanticView(bool shown)
    {
        (void)shown;
        return false;
    }
    virtual void PullSemanticViewToFront() {}

protected:
    virtual BOOL HandleMessage()
    {
        return FALSE;
    }

    DWORD m_dwUIID;
    DWORD m_dwParentUIID = 0;
    int m_iState = UISTATE_NORMAL;
    int m_iPos_x = 0, m_iPos_y = 0;
    int m_iWidth = 100, m_iHeight = 100;
    std::wstring m_strTitle;
    std::wstring m_returnText;
    BOOL m_bHaveTextBox;
    BOOL m_bIsMaximize;
    int m_iBackPos_y, m_iBackHeight;
};
