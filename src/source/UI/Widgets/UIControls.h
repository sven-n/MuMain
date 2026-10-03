#ifndef __UICONTROL_H__
#define __UICONTROL_H__

#include "Engine/Object/ZzzInfomation.h"

#include "Network/Server/WSclient.h"
#include "GameLogic/Quests/QuestMng.h"
#include "Core/Time/Timer.h"
#include <limits>
#include <type_traits>
#include <memory>
#include <vector>

inline DWORD _ARGB(BYTE a, BYTE r, BYTE g, BYTE b)
{
    return (a << 24) + (b << 16) + (g << 8) + (r);
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#define UIMAX_TEXT_LINE 150
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

void RenderCheckBox(int iPos_x, int iPos_y, BOOL bFlag);

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

enum UIOPTIONS
{
    UIOPTION_NULL = 0,
    UIOPTION_NUMBERONLY = 1,
    UIOPTION_SERIALNUMBER = 2,
    UIOPTION_ENTERIMECHKOFF = 4,
    UIOPTION_PAINTBACK = 8,
    UIOPTION_NOLOCALIZEDCHARACTERS = 16
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
    UI_MESSAGE_P_MOVE,
    UI_MESSAGE_P_RESIZE,
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
    // cppcheck-suppress virtualCallInConstructor ; called from the constructor, static binding intended
    virtual void SetArrangeType(int iArrangeType = 0, int iRelativePos_x = 0, int iRelativePos_y = 0);
    // cppcheck-suppress virtualCallInConstructor ; called from the constructor, static binding intended
    virtual void SetResizeType(int iResizeType = 0, int iRelativeWidth = 0, int iRelativeHeight = 0);
    virtual void Render() {}
    virtual BOOL DoAction(BOOL bMessageOnly = FALSE);

protected:
    virtual void DoActionSub(BOOL bMessageOnly) {}
    virtual BOOL DoMouseAction()
    {
        return TRUE;
    }
    virtual void DefaultHandleMessage();
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
    int m_iArrangeType;
    int m_iResizeType;
    int m_iRelativePos_x, m_iRelativePos_y;
    int m_iRelativeWidth, m_iRelativeHeight;
    int m_iCoordType;
};

enum UILISTBOX_SCROLL_TYPE
{
    UILISTBOX_SCROLL_DOWNUP = 0,
    UILISTBOX_SCROLL_UPDOWN
};

// A list box's scroll bar in reference px: its track and its thumb's top.
struct TextListScrollBarGeometry
{
    float rangeTop = 0.f;
    float rangeBottom = 0.f;
    float thumbTop = 0.f;
    bool dragged = false;
    float thumbHeight = 0.f; // ComputeLegacyScrollBar() only
    float barWidth = 0.f;
};

template <class T> class CUITextListBox : public CUIControl
{
public:
    CUITextListBox();
    virtual ~CUITextListBox();

    // cppcheck-suppress virtualCallInConstructor ; called from the constructor/destructor, static binding intended
    virtual void Clear();
    virtual void AddText() {}

    virtual void Render();
    virtual void Scrolling(int iValue);

    virtual int GetBoxSize()
    {
        return m_iNumRenderLine;
    }
    virtual void SetBoxSize(int iLineNum)
    {
        m_iNumRenderLine = iLineNum;
    }
    virtual void SetNumRenderLine(int iLine)
    {
        m_iNumRenderLine = iLine;
    }
    virtual void Resize(int iValue);
    virtual BOOL HandleMessage();

    virtual void ResetCheckedLine(BOOL bFlag = FALSE);
    virtual BOOL HaveCheckedLine();
    virtual int GetCheckedLines(std::deque<T*>* pSelectLineList);
    virtual int GetLineNum()
    {
        return (m_bUseMultiline == TRUE ? m_RenderTextList.size() : m_TextList.size());
    }

    // cppcheck-suppress virtualCallInConstructor ; called from the constructor/destructor, static binding intended
    virtual void SLSetSelectLine(int iLineNum);
    virtual void SLSelectPrevLine(int iLineNum = 1);
    virtual void SLSelectNextLine(int iLineNum = 1);
    virtual typename std::deque<T>::iterator SLGetSelectLine();

    virtual int SLGetSelectLineNum()
    {
        return m_iSelectLineNum;
    }

    // The lines Render() draws (a multiline list's wrapped ones), in its order: visit(line, item,
    // selected) with line 0 the first drawn. For windows that draw the list elsewhere (RmlUi) and keep
    // this control for its data, scrolling and line clicks. A visit returning false skips the item
    // without using up its line, as a RenderDataLine() returning FALSE does.
    template <typename Visit> void ForEachRenderLine(Visit&& visit)
    {
        MoveRenderLine();
        for (int i = 0; i < m_iNumRenderLine; ++i, ++m_TextListIter)
        {
            if (m_TextListIter == (m_bUseMultiline == TRUE ? m_RenderTextList.end() : m_TextList.end()))
                break;
            const bool selected = SLGetSelectLineNum() == m_iCurrentRenderEndLine + i + 1;
            if constexpr (std::is_void_v<decltype(visit(i, *m_TextListIter, selected))>)
                visit(i, *m_TextListIter, selected);
            else if (!visit(i, *m_TextListIter, selected))
                --i;
        }
    }

    // The new-style scroll bar RenderInterface() draws (m_bUseNewUIScrollBar): computed as the
    // render does unless the thumb is being dragged.
    TextListScrollBarGeometry GetScrollBarGeometry()
    {
        const bool dragged = GetState() == UISTATE_SCROLL;
        if (!dragged)
            ComputeScrollBar();
        return {m_fScrollBarRange_top, m_fScrollBarRange_bottom, m_fScrollBarPos_y, dragged};
    }

    // The old-style scroll bar the friends family's lists draw: RenderInterface() computes it every
    // frame, also while the thumb is dragged.
    TextListScrollBarGeometry ComputeLegacyScrollBar()
    {
        ComputeScrollBar();
        return {m_fScrollBarRange_top,        m_fScrollBarRange_bottom, m_fScrollBarPos_y,
                GetState() == UISTATE_SCROLL, m_fScrollBarHeight,       m_fScrollBarWidth};
    }

protected:
    virtual BOOL DoMouseAction();
    virtual void RemoveText();
    virtual void ComputeScrollBar();
    virtual void MoveRenderLine();
    virtual BOOL CheckMouseInBox();

    virtual void RenderInterface() = 0;
    virtual BOOL RenderDataLine(int iLineNumber) = 0;
    virtual void RenderCoveredInterface() {}
    virtual BOOL DoLineMouseAction(int iLineNumber) = 0;
    virtual BOOL DoSubMouseAction()
    {
        return TRUE;
    }

    virtual void CalcLineNum() {}

protected:
    std::deque<T> m_TextList;
    typename std::deque<T>::iterator m_TextListIter;

    BOOL m_bUseSelectLine;
    BOOL m_bPressCursorKey;
    int m_iSelectLineNum;

    BOOL m_bUseMultiline;
    std::deque<T> m_RenderTextList;

    int m_iMaxLineCount;
    int m_iCurrentRenderEndLine;
    int m_iNumRenderLine;

    float m_fScrollBarRange_top;
    float m_fScrollBarRange_bottom;

    float m_fScrollBarPos_y;
    float m_fScrollBarWidth;
    float m_fScrollBarHeight;

    float m_fScrollBarClickPos_y;
    BOOL m_bScrollBtnClick;
    BOOL m_bScrollBarClick;

    int m_iScrollType;
    BOOL m_bNewTypeScrollBar;

    BOOL m_bUseNewUIScrollBar;
};

struct InputBoxConfig
{
    POINT pos = {std::numeric_limits<int>::min(), std::numeric_limits<int>::min()};
    SIZE size = {0, 0};
    int textLimit = MAX_TEXT_LENGTH;
    DWORD options = UIOPTION_NULL;
    bool password = false;
    BYTE textAlpha = 255;
    BYTE textR = 0;
    BYTE textG = 0;
    BYTE textB = 0;
    BYTE backAlpha = 0;
    BYTE backR = 0;
    BYTE backG = 0;
    BYTE backB = 0;
    BYTE selectAlpha = 255;
    BYTE selectR = 255;
    BYTE selectG = 255;
    BYTE selectB = 255;
    HFONT font = nullptr;
    int state = UISTATE_NORMAL;
};

class CUITextInputBox : public CUIControl
{
public:
    CUITextInputBox();
    virtual ~CUITextInputBox();

    virtual void SetSize(int iWidth, int iHeight);

    virtual void Init(HWND hWnd, int iWidth, int iHeight, int iMaxLength = 50, BOOL bIsPassword = FALSE);
    virtual void Render();
    virtual void GiveFocus(BOOL bSel = FALSE);

    virtual void SetState(int iState);
    virtual void SetFont(HFONT hFont);
    virtual void SetMultiline(BOOL bUseFlag)
    {
        m_bUseMultiLine = bUseFlag;
    }

    virtual void SetTextLimit(int iLimit);
    virtual void SetTextColor(BYTE a, BYTE r, BYTE g, BYTE b)
    {
        m_dwTextColor = _ARGB(a, r, g, b);
    }
    virtual void SetBackColor(BYTE a, BYTE r, BYTE g, BYTE b)
    {
        m_dwBackColor = _ARGB(a, r, g, b);
    }
    virtual void SetSelectBackColor(BYTE a, BYTE r, BYTE g, BYTE b)
    {
        m_dwSelectBackColor = _ARGB(a, r, g, b);
    }
    virtual void SetText(const wchar_t* pszText);
    virtual void GetText(wchar_t* pszText, int iGetLength = MAX_TEXT_LENGTH);
    virtual void Reset();
    virtual void Configure(const InputBoxConfig& config);
    virtual void SetIsPassword(bool isPassword)
    {
        m_bPasswordInput = isPassword ? TRUE : FALSE;
    }

    // Stable per-instance token used only as a focus identity for NewUI "related window" routing (never messaged); there is no real Win32 EDIT child.
    HWND GetHandle()
    {
        return reinterpret_cast<HWND>(this);
    }
    HWND GetParentHandle()
    {
        return m_hParentWnd;
    }
    BOOL HaveFocus()
    {
        return s_pFocusedPortable == this;
    }
    BOOL UseMultiline()
    {
        return m_bUseMultiLine;
    }
    virtual void SetTabTarget(CUITextInputBox* pTabTarget)
    {
        m_pTabTarget = pTabTarget;
    }
    CUITextInputBox* GetTabTarget()
    {
        return m_pTabTarget;
    }

    virtual void Lock(BOOL bFlag)
    {
        m_bLock = bFlag;
    }
    virtual BOOL IsLocked()
    {
        return m_bLock;
    }
    BOOL IsPassword()
    {
        return m_bPasswordInput;
    }

#ifdef PBG_ADD_INGAMESHOPMSGBOX
    bool GetUseScrollbar()
    {
        return m_bUseScrollbarRender;
    }
    void SetUseScrollbar(bool _scrollbar = TRUE)
    {
        m_bUseScrollbarRender = _scrollbar;
    }
#endif // PBG_ADD_INGAMESHOPMSGBOX

    // The single text field currently owning keyboard input, or nullptr; the SDL event loop routes text/edit keys to it.
    static CUITextInputBox* GetFocusedPortable()
    {
        return s_pFocusedPortable;
    }
    static bool IsAnyInputBoxFocused()
    {
        return s_pFocusedPortable != nullptr && s_pFocusedPortable->m_iState != UISTATE_HIDE;
    }
    static bool IsFocusedForParent(DWORD parentUIID)
    {
        return IsAnyInputBoxFocused() && s_pFocusedPortable->GetParentUIID() == parentUIID;
    }

    // Releases keyboard focus from the focused field (counterpart to GiveFocus()); the field stays visible.
    static void ReleaseFocus();

    // Input fed from the SDL event loop.
    void OnTextInput(const wchar_t* pszText);                 // committed characters
    void OnTextEditing(const wchar_t* pszText);               // IME composition preview (uncommitted)
    void OnEditKey(int iVirtualKey, bool bCtrl, bool bShift); // navigation/erase
    void SelectAll();
    std::wstring GetSelectedText() const;
    void DeleteSelection();

    // Caret rect in reference pixels for positioning the IME candidate window; false if not the focused field or not yet rendered.
    bool GetCaretArea(int& x, int& y, int& w, int& h) const;

    // For a window that shows this field as an RmlUi <input> (UI/Party/FriendWindowView.h): the
    // value, caret and look it would render, and the value typed there (cut to the text limit,
    // caret at its end; unlike SetText() also longer than MAX_TEXT_LENGTH, as typing allows).
    const std::wstring& GetValue() const
    {
        return m_portableText;
    }
    void SetValueFromField(const std::wstring& value)
    {
        m_portableText = value;
        if (m_iMaxLength > 0 && static_cast<int>(m_portableText.length()) > m_iMaxLength)
            m_portableText.resize(m_iMaxLength);
        m_iCaret = static_cast<int>(m_portableText.length());
        m_iSelAnchor = m_iCaret;
        m_iFirstVisible = 0;
        m_composition.clear();
    }
    int GetCaret() const
    {
        return m_iCaret;
    }
    int GetSelectionAnchor() const
    {
        return m_iSelAnchor;
    }
    int GetTextLimit() const
    {
        return m_iMaxLength;
    }
    DWORD GetTextColor() const
    {
        return m_dwTextColor;
    }
    DWORD GetBackColor() const
    {
        return m_dwBackColor;
    }
    DWORD GetSelectBackColor() const
    {
        return m_dwSelectBackColor;
    }

protected:
    virtual BOOL DoMouseAction();

    // Portable text field implementation.
    struct PortableLine
    {
        int start;
        int end;
    }; // [start,end) buffer indices; end excludes a wrapped space/newline

    void RenderPortable();
    BOOL DoPortableMouse();
    std::wstring BuildDisplay() const; // text with the password mask applied
    // Renders display text with the caret and an underlined IME composition span [compStart, compEnd) (compStart < 0 = none).
    void RenderPortableSingleLine(const std::wstring& display, int iCaret, int iLineHeight, int compStart, int compEnd);
    void RenderPortableMultiline(const std::wstring& display, int iCaret, int iLineHeight, int compStart, int compEnd);
    void RenderPortableScrollbar(int iTotalLines, int iVisibleLines);
    void LayoutLines(const std::wstring& display, std::vector<PortableLine>& lines) const;
    int CaretToLine(const std::vector<PortableLine>& lines, int iCaret) const;
    // Buffer index on a line whose rendered x (reference px) is nearest targetX.
    int IndexAtLineX(const std::wstring& display, const PortableLine& line, int targetX) const;
    int LineHeight() const;
    int VisibleLineCount(int iLineHeight) const;
    void MoveCaret(int iNewCaret, bool bExtendSelection);
    void InsertChar(wchar_t ch);
    bool HasSelection() const
    {
        return m_iSelAnchor != m_iCaret;
    }
    int SelectionStart() const
    {
        return m_iSelAnchor < m_iCaret ? m_iSelAnchor : m_iCaret;
    }
    int SelectionEnd() const
    {
        return m_iSelAnchor < m_iCaret ? m_iCaret : m_iSelAnchor;
    }
    // Width in reference pixels of the first iLength chars rendered in the font.
    int MeasureWidth(const wchar_t* pszText, int iLength) const;
    HFONT CurrentFont() const; // live handle for m_fontKind

public:
    CTimer m_caretTimer = {};

protected:
    HWND m_hParentWnd;

    // Portable text field state.
    std::wstring m_portableText;
    std::wstring m_composition; // IME preedit shown at the caret, not yet committed
    int m_iCaretAreaX = 0;      // last rendered caret rect (reference px, screen space)
    int m_iCaretAreaY = 0;      // for positioning the IME candidate window
    int m_iCaretAreaH = 0;
    int m_iCaret = 0;        // caret index in [0, length]
    int m_iSelAnchor = 0;    // selection anchor; equals caret when no selection
    int m_iFirstVisible = 0; // first rendered character (single-line horizontal scroll)
    int m_iScrollLine = 0;   // first visible wrapped line (multiline vertical scroll)
    int m_iMaxLength = 0;    // text length limit (0 = unlimited)
    // Font stored by kind, not HFONT -- ReinitializeFonts() recreates handles on font-family change, so CurrentFont() re-resolves each frame.
    enum class PortableFontKind
    {
        Default,
        Bold,
        Big,
        Fix
    };
    PortableFontKind m_fontKind = PortableFontKind::Default;
    static CUITextInputBox* s_pFocusedPortable;

    CUITextInputBox* m_pTabTarget;

    DWORD m_dwTextColor;
    DWORD m_dwBackColor;
    DWORD m_dwSelectBackColor;

    BOOL m_bPasswordInput;
    BOOL m_bLock;

    BOOL m_bUseMultiLine;
    BOOL m_bScrollBtnClick;
    BOOL m_bScrollBarClick;
    int m_iNumLines;
    float m_fScrollBarWidth;
    float m_fScrollBarRange_top;
    float m_fScrollBarRange_bottom;
    float m_fScrollBarHeight;
    float m_fScrollBarPos_y;
    float m_fScrollBarClickPos_y;
#ifdef PBG_ADD_INGAMESHOPMSGBOX
    bool m_bUseScrollbarRender;
#endif // PBG_ADD_INGAMESHOPMSGBOX
};



extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;


#endif //__UICONTROL_H__
