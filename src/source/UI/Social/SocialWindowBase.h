#pragma once

// The base class of every window in the friend/mail/chat family, and the enums its manager
// addresses them with. The windows themselves are RmlUi documents; what this carries is the
// bookkeeping CUIWindowMgr runs them through -- identity, state, geometry, draw order.
//
// Still derives from CUIControl (SocialWindowCore.h), which is the only reason that file exists.

#include "UI/Social/SocialWindowCore.h"
#include <memory>
#include <string>
#include <vector>

class CUIPhotoViewer;
namespace UI::Social { class FriendShell; class ChatRoomView; class LetterReadView; class LetterWriteView; }

class CUIBaseWindow : public CUIControl
{
public:
    CUIBaseWindow();
    virtual ~CUIBaseWindow();

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
    std::wstring m_strTitle;
    std::wstring m_returnText;
    BOOL m_bHaveTextBox;
    BOOL m_bIsMaximize;
    int m_iBackPos_y, m_iBackHeight;
};
