#pragma once

// What is left of the old UI/Social/SocialWindowCore.h widget toolkit: the base every window of the
// friend/mail/chat family sits on, and the UI-message queue its manager runs them through.
//
// This is not a general widget toolkit any more and should not be used as one. Every widget that
// made it one -- the buttons, the list boxes, the text input -- is deleted, and new UI is built
// from RmlUi documents (docs/rmlui-ui-system/building-new-ui.md). It lives beside its only users
// in UI/Social/ rather than in UI/Widgets/ so that is plain from the path.
//
// CUIControl carries a window's identity, state, geometry and options; CUIMessage carries the
// queue. Taking CUIBaseWindow and CUIPhotoViewer off this base is what would delete the file.

#include "Engine/Object/ZzzInfomation.h"

#include "Network/Server/WSclient.h"
#include "GameLogic/Quests/QuestMng.h"
#include "Core/Time/Timer.h"
#include <limits>
#include <type_traits>
#include <memory>
#include <vector>

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

class CUIControl : public CUIMessage
{
public:
    CUIControl();
    virtual ~CUIControl() {}

    DWORD GetUIID()
    {
        return m_dwUIID;
    }
    void SetParentUIID(DWORD dwParentUIID)
    {
        m_dwParentUIID = dwParentUIID;
    }
    DWORD GetParentUIID()
    {
        return m_dwParentUIID;
    }
    // cppcheck-suppress virtualCallInConstructor ; called from the constructor, static binding intended
    virtual void SetState(int iState);
    int GetState();
    void SetOption(int iOption)
    {
        m_iOptions = iOption;
    }
    BOOL CheckOption(int iOption)
    {
        return m_iOptions & iOption;
    }

    void SendUIMessageDirect(int iMessage, int iParam1, int iParam2);

    void SetPosition(int iPos_x, int iPos_y);
    int GetPosition_x()
    {
        return m_iPos_x;
    }
    int GetPosition_y()
    {
        return m_iPos_y;
    }
    // cppcheck-suppress virtualCallInConstructor ; called from the constructor, static binding intended
    virtual void SetSize(int iWidth, int iHeight);
    int GetWidth()
    {
        return m_iWidth;
    }
    int GetHeight()
    {
        return m_iHeight;
    }
    virtual void Render() {}
    virtual BOOL DoAction(BOOL bMessageOnly = FALSE);

protected:
    virtual void DoActionSub(BOOL bMessageOnly) {}
    virtual BOOL DoMouseAction()
    {
        return TRUE;
    }
    virtual BOOL HandleMessage()
    {
        return FALSE;
    }

protected:
    DWORD m_dwUIID;
    DWORD m_dwParentUIID;
    int m_iState;
    int m_iOptions;
    int m_iPos_x, m_iPos_y;
    int m_iWidth, m_iHeight;
};

extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;

