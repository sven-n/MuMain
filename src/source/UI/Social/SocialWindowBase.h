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

const int g_ciWindowFrameThickness = 5;
const int g_ciWindowTitleHeight = 21;

enum UIWINDOWSTYLE

{
    UIWINDOWSTYLE_NULL = 0,
    UIWINDOWSTYLE_TITLEBAR = 1,
    UIWINDOWSTYLE_FRAME = 2,
    UIWINDOWSTYLE_RESIZEABLE = 4,
    UIWINDOWSTYLE_MOVEABLE = 8,
    UIWINDOWSTYLE_MINBUTTON = 16,
    UIWINDOWSTYLE_MAXBUTTON = 32,
    UIWINDOWSTYLE_NORMAL = UIWINDOWSTYLE_TITLEBAR | UIWINDOWSTYLE_FRAME | UIWINDOWSTYLE_RESIZEABLE |
                           UIWINDOWSTYLE_MOVEABLE | UIWINDOWSTYLE_MINBUTTON | UIWINDOWSTYLE_MAXBUTTON,
    UIWINDOWSTYLE_FIXED = UIWINDOWSTYLE_TITLEBAR | UIWINDOWSTYLE_FRAME | UIWINDOWSTYLE_MOVEABLE
};

class CUIBaseWindow : public CUIControl
{
public:
    CUIBaseWindow();
    virtual ~CUIBaseWindow();

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);

    virtual void Refresh() {}
    void SetLimitSize(int iMinWidth, int iMinHeight, int iMaxWidth = 0, int iMaxHeight = 0)
    {
        m_iMinWidth = iMinWidth;
        m_iMinHeight = iMinHeight;
        m_iMaxWidth = iMaxWidth;
        m_iMaxHeight = iMaxHeight;
    }
    virtual void SetTitle(const wchar_t* pszTitle);
    void SetReturnText(const wchar_t* text);
    std::wstring TakeReturnText();
    const wchar_t* GetTitle()
    {
        return m_strTitle.c_str();
    }
    virtual void Maximize();

    void Render();
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


    // CUIWindowMgr::Render() for a window with an RmlUi view: RenderOver() only.
    void RenderRmlOverlay();
    // Native 3D this window owns, drawn after RmlUi's main context has composited rather than
    // before it -- otherwise every panel in the frame paints over it. Reached through
    // CUIWindowMgr::RenderOverlay3D(), and only while this window is the one in front: see there
    // for why the others draw nothing.
    virtual void RenderAboveRmlUi() {}
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
    BOOL DoMouseAction();
    virtual void InitControls() = 0;

    virtual void RenderSub() {}
    virtual void RenderOver() {}
    virtual void DoActionSub(BOOL bMessageOnly) {}
    virtual void DoMouseActionSub() {}
    void DrawOutLine(int iPos_x, int iPos_y, int iWidth, int iHeight);
    void SetControlButtonColor(int iSelect);

protected:
    int m_iMouseClickPos_x, m_iMouseClickPos_y;
    int m_iResizeDir;
    int m_iMinWidth, m_iMinHeight;
    int m_iMaxWidth, m_iMaxHeight;
    std::wstring m_strTitle;
    std::wstring m_returnText;
    BOOL m_bHaveTextBox;
    int m_iControlButtonClick;
    BOOL m_bIsMaximize;
    int m_iBackPos_y, m_iBackHeight;

    std::once_flag _controlsInitialized;
};
