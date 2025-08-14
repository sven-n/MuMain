#ifndef __UICONTROL_H__
#define __UICONTROL_H__

#include "zzzinfomation.h"

#include "WSclient.h"
#include "Time/Timer.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#define UIMAX_TEXT_LINE			150
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

inline DWORD _ARGB(BYTE a, BYTE r, BYTE g, BYTE b) { return (a << 24) + (b << 16) + (g << 8) + (r); }

int CutStr(const wchar_t* pszText, wchar_t* pTextOut, const int iTargetPixelWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab = 0);
int CutText3(const wchar_t* pszText, wchar_t* pTextOut, const int TargetWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab = 0, const BOOL bReverseWrite = FALSE);
void CutText4(const wchar_t* pszSource, wchar_t* pszResult1, wchar_t* pszResult2, int iCutCount);

void RenderCheckBox(int iPos_x, int iPos_y, BOOL bFlag);

const int COORDINATE_TYPE_LEFT_TOP = 1;
const int COORDINATE_TYPE_LEFT_DOWN = 2;

BOOL CheckMouseIn(int iPos_x, int iPos_y, int iWidth, int iHeight, int CoordType = COORDINATE_TYPE_LEFT_TOP);

#define ID_UICEDIT 0x0001

enum UISTATES
{
    UISTATE_NORMAL = 0, UISTATE_RESIZE, UISTATE_SCROLL, UISTATE_HIDE,
    UISTATE_MOVE, UISTATE_READY, UISTATE_DISABLE
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
    BOOL	m_bIsSelected;
    wchar_t	m_szID[MAX_ID_SIZE + 1];
    wchar_t	m_szText[MAX_TEXT_LENGTH + 1];
    int		m_iType;
    int		m_iColor;
    UINT	m_uiEmptyLines;
} WHISPER_TEXT;

typedef struct
{
    BOOL	m_bIsSelected;
    wchar_t	m_szID[MAX_ID_SIZE + 1];
    BYTE	m_Number;
    BYTE	m_Server;
    BYTE	m_GuildStatus;
} GUILDLIST_TEXT;

typedef struct
{
    BOOL    m_bIsSelected;
    wchar_t	m_szPattern[MAX_ITEM_NAME + 1];
} FILTERLIST_TEXT;

typedef struct
{
    BOOL	m_bIsSelected;
    DWORD	m_dwUIID;
    wchar_t	m_szTitle[64];
    int		m_iStatus;
} WINDOWLIST_TEXT;

typedef struct
{
    BOOL	m_bIsSelected;
    DWORD	m_dwLetterID;
    wchar_t	m_szID[MAX_ID_SIZE + 1];
    wchar_t	m_szText[MAX_TEXT_LENGTH + 1];
    wchar_t	m_szDate[16];
    wchar_t	m_szTime[16];
    BOOL	m_bIsRead;
} LETTERLIST_TEXT;

typedef struct
{
    BOOL	m_bIsSelected;
    wchar_t	m_szText[MAX_LETTERTEXT_LENGTH + 1];
} LETTER_TEXT;

typedef struct
{
    BOOL	m_bIsSelected;
    wchar_t	m_szContent[60];
} GUILDLOG_TEXT;
typedef struct
{
    BOOL	m_bIsSelected;
    BYTE	GuildMark[64];
    wchar_t	szName[MAX_GUILDNAME + 1];
    int		nMemberCount;
} UNIONGUILD_TEXT;

typedef struct
{
    BOOL	m_bIsSelected;
    wchar_t	szName[MAX_GUILDNAME + 1];
    int		nCount;
    BYTE	byIsGiveUp;
    BYTE	bySeqNum;
} BCDECLAREGUILD_TEXT;

typedef struct tagMOVECOMMAND_TEXT
{
    BOOL	m_bIsSelected;
    BOOL	m_bCanMove;
    wchar_t	szMainMapName[32];		//. Main map name
    wchar_t	szSubMapName[32];		//. Substitute map name
    int		iReqLevel;				//. required level
    int		iReqZen;				//. required zen
    int		iGateNum;				//. Gate number
} MOVECOMMAND_TEXT;

typedef struct
{
    BOOL	m_bIsSelected;
    wchar_t	szName[MAX_GUILDNAME + 1];
    BYTE	byJoinSide;
    BYTE	byGuildInvolved;
    int		iGuildScore;
} BCGUILD_TEXT;

typedef struct _UNMIX_TEXT
{
    BOOL	m_bIsSelected;
    int		m_iInvenIdx;
    char	m_cLevel;

    _UNMIX_TEXT() : m_bIsSelected(false), m_iInvenIdx(-1), m_cLevel(-1) {}
}UNMIX_TEXT;

typedef struct _SOCKETLIST_TEXT
{
    BOOL	m_bIsSelected;
    int		m_iSocketIndex;
    wchar_t	m_szText[64 + 1];
} SOCKETLIST_TEXT;

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
    int m_iParam1;
    int m_iParam2;
};

class CUIMessage
{
public:
    CUIMessage() {}
    virtual ~CUIMessage() { m_MessageList.clear(); }

    void SendUIMessage(int iMessage, int iParam1, int iParam2);
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

    DWORD GetUIID() { return m_dwUIID; }
    void SetParentUIID(DWORD dwParentUIID) { m_dwParentUIID = dwParentUIID; }
    DWORD GetParentUIID() { return m_dwParentUIID; }
    virtual void SetState(int iState);
    int GetState();
    void SetOption(int iOption) { m_iOptions = iOption; }
    BOOL CheckOption(int iOption) { return m_iOptions & iOption; }

    void SendUIMessageDirect(int iMessage, int iParam1, int iParam2);

    void SetPosition(int iPos_x, int iPos_y);
    int GetPosition_x() { return m_iPos_x; }
    int GetPosition_y() { return m_iPos_y; }
    virtual void SetSize(int iWidth, int iHeight);
    int GetWidth() { return m_iWidth; }
    int GetHeight() { return m_iHeight; }
    virtual void SetArrangeType(int iArrangeType = 0, int iRelativePos_x = 0, int iRelativePos_y = 0);
    virtual void SetResizeType(int iResizeType = 0, int iRelativeWidth = 0, int iRelativeHeight = 0);
    virtual void Render() {}
    virtual BOOL DoAction(BOOL bMessageOnly = FALSE);

protected:
    virtual void DoActionSub(BOOL bMessageOnly) {			}
    virtual BOOL DoMouseAction() { return TRUE; }
    virtual void DefaultHandleMessage();
    virtual BOOL HandleMessage() { return FALSE; }

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

class CUIButton : public CUIControl
{
public:
    CUIButton();
    virtual ~CUIButton();

    virtual void Init(DWORD dwButtonID, const wchar_t* pszCaption);
    virtual void SetCaption(const wchar_t* pszCaption);
    virtual BOOL DoMouseAction();
    virtual void Render();

protected:
    DWORD m_dwButtonID;
    wchar_t* m_pszCaption;
    BOOL m_bMouseState;
};

enum UILISTBOX_SCROLL_TYPE { UILISTBOX_SCROLL_DOWNUP = 0, UILISTBOX_SCROLL_UPDOWN };

template <class T>
class CUITextListBox : public CUIControl
{
public:
    CUITextListBox();
    virtual ~CUITextListBox();

    virtual void Clear();
    virtual void AddText() {}

    virtual void Render();
    virtual void Scrolling(int iValue);

    virtual int GetBoxSize() { return m_iNumRenderLine; }
    virtual void SetBoxSize(int iLineNum) { m_iNumRenderLine = iLineNum; }
    virtual void SetNumRenderLine(int iLine) { m_iNumRenderLine = iLine; }
    virtual void Resize(int iValue);
    virtual BOOL HandleMessage();

    virtual void ResetCheckedLine(BOOL bFlag = FALSE);
    virtual BOOL HaveCheckedLine();
    virtual int GetCheckedLines(std::deque<T*>* pSelectLineList);
    virtual int GetLineNum() { return (m_bUseMultiline == TRUE ? m_RenderTextList.size() : m_TextList.size()); }

    virtual void SLSetSelectLine(int iLineNum);
    virtual void SLSelectPrevLine(int iLineNum = 1);
    virtual void SLSelectNextLine(int iLineNum = 1);
    virtual typename std::deque<T>::iterator SLGetSelectLine();

    virtual int SLGetSelectLineNum() { return m_iSelectLineNum; }

protected:
    virtual BOOL DoMouseAction();
    virtual void RemoveText();
    virtual void ComputeScrollBar();
    virtual void MoveRenderLine();
    virtual BOOL CheckMouseInBox();

    virtual void RenderInterface() = 0;
    virtual BOOL RenderDataLine(int iLineNumber) = 0;
    virtual void RenderCoveredInterface() { }
    virtual BOOL DoLineMouseAction(int iLineNumber) = 0;
    virtual BOOL DoSubMouseAction() { return TRUE; }

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

class CUIGuildListBox : public CUITextListBox<GUILDLIST_TEXT>
{
public:
    CUIGuildListBox();
    virtual ~CUIGuildListBox() {}

    virtual void AddText(const wchar_t* pszID, BYTE Number, BYTE Server);
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber) { return TRUE; }
    virtual BOOL DoSubMouseAction();
protected:
    BOOL m_bIsGuildMaster;
};

class CUISimpleChatListBox : public CUITextListBox<WHISPER_TEXT>
{
public:
    CUISimpleChatListBox();
    virtual ~CUISimpleChatListBox() {}

    virtual void Render();
    virtual void AddText(const wchar_t* pszID, const wchar_t* pszText, int iType, int iColor);
protected:
    virtual void AddTextToRenderList(const wchar_t* pszID, const wchar_t* pszText, int iType, int iColor);
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber) { return TRUE; }
    void CalcLineNum();
};

class CUILetterTextListBox : public CUITextListBox<LETTER_TEXT>
{
public:
    CUILetterTextListBox();
    virtual ~CUILetterTextListBox() {}

    virtual void Render();
    virtual void AddText(const wchar_t* pszText);
protected:
    virtual void AddTextToRenderList(const wchar_t* pszText);
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber) { return TRUE; }
    void CalcLineNum();
    virtual int	GetRenderLinePos_y(int iLineNumber);
};

class CUIChatPalListBox : public CUITextListBox<GUILDLIST_TEXT>
{
public:
    CUIChatPalListBox();
    virtual ~CUIChatPalListBox() {}

    virtual void AddText(const wchar_t* pszID, BYTE Number, BYTE Server);
    virtual void DeleteText(const wchar_t* pszID);
    virtual void SetNumRenderLine(int iLine);
    GUILDLIST_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
    std::deque<GUILDLIST_TEXT>& GetFriendList() { m_bForceEditList = TRUE; return m_TextList; }
    void SetLayout(int iType) { m_iLayoutType = iType; }
    const wchar_t* GetNameByNumber(BYTE byNumber);
    void SetColumnWidth(UINT uiColumnNum, int iWidth) { if (uiColumnNum < 4) m_iColumnWidth[uiColumnNum] = iWidth; }
    int GetColumnWidth(UINT uiColumnNum) { return (uiColumnNum < 4 ? m_iColumnWidth[uiColumnNum] : 0); }
    int GetColumnPos_x(UINT uiColumnNum)
    {
        int iResult = 0;

        for (unsigned int i = 0; i < uiColumnNum; ++i)
        {
            iResult += m_iColumnWidth[i];
        }
        return iResult;
    }
    void MakeTitleText(wchar_t* pszTitleText);

protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);

protected:
    int m_iLayoutType;
    int m_iColumnWidth[4];
    BOOL m_bForceEditList;
};

class CUIWindowListBox : public CUITextListBox<WINDOWLIST_TEXT>
{
public:
    CUIWindowListBox();
    virtual ~CUIWindowListBox() {}

    virtual void AddText(DWORD dwUIID, const wchar_t* pszTitle, int iStatus = 0);
    virtual void DeleteText(DWORD dwUIID);
    virtual void SetNumRenderLine(int iLine);
    WINDOWLIST_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);
};

class CUILetterListBox : public CUITextListBox<LETTERLIST_TEXT>
{
public:
    CUILetterListBox();
    virtual ~CUILetterListBox() {}

    virtual void AddText(const wchar_t* pszID, const wchar_t* pszText, const wchar_t* pszDate, const wchar_t* pszTime, BOOL bIsRead);
    virtual void DeleteText(DWORD dwLetterID);
    virtual void SetNumRenderLine(int iLine);
    LETTERLIST_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
    std::deque<LETTERLIST_TEXT>& GetLetterList() { m_bForceEditList = TRUE; return m_TextList; }

    void SetColumnWidth(UINT uiColumnNum, int iWidth) { if (uiColumnNum < 4) m_iColumnWidth[uiColumnNum] = iWidth; }
    int GetColumnWidth(UINT uiColumnNum) { return (uiColumnNum < 4 ? m_iColumnWidth[uiColumnNum] : 0); }
    int GetColumnPos_x(UINT uiColumnNum)
    {
        int iResult = 0;
        for (unsigned int i = 0; i < uiColumnNum; ++i)
        {
            iResult += m_iColumnWidth[i];
        }
        return iResult;
    }
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);

protected:
    int m_iColumnWidth[4];
    BOOL m_bForceEditList;
};

class CUISocketListBox : public CUITextListBox<SOCKETLIST_TEXT>
{
public:
    CUISocketListBox();
    virtual ~CUISocketListBox() {}

    virtual void AddText(int iSocketIndex, const wchar_t* pszText);
    virtual void DeleteText(int iSocketIndex);
    virtual void SetNumRenderLine(int iLine);
    SOCKETLIST_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);
};

class CUIGuildNoticeListBox : public CUITextListBox<GUILDLOG_TEXT>
{
public:
    CUIGuildNoticeListBox();
    virtual ~CUIGuildNoticeListBox() {}

    virtual void AddText(const wchar_t* szContent);
    virtual void DeleteText(DWORD dwIndex);
    virtual void SetNumRenderLine(int nLine);
    GUILDLOG_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);
};

class CUINewGuildMemberListBox : public CUITextListBox<GUILDLIST_TEXT>
{
public:
    CUINewGuildMemberListBox();
    virtual ~CUINewGuildMemberListBox() {}

    virtual void AddText(const wchar_t* pszID, BYTE Number, BYTE Server, BYTE GuildStatus);
    virtual void DeleteText(DWORD dwUIID);
    virtual void SetNumRenderLine(int iLine);
    GUILDLIST_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);
protected:
    BOOL m_bIsGuildMaster;
};

class CUIUnionGuildListBox : public CUITextListBox<UNIONGUILD_TEXT>

{
public:
    CUIUnionGuildListBox();
    virtual ~CUIUnionGuildListBox() {}

    virtual void AddText(BYTE* pGuildMark, const wchar_t* szGuildName, int nMemberCount);
    virtual void DeleteText(DWORD dwGuildIndex);
    virtual int GetTextCount();
    virtual void SetNumRenderLine(int iLine);
    UNIONGUILD_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);
};

class CUIExtraItemListBox : public CUITextListBox<FILTERLIST_TEXT>
{
public:
    CUIExtraItemListBox();
    ~CUIExtraItemListBox() = default;

    virtual void AddText(const wchar_t* pszPattern);
    virtual void DeleteText(const wchar_t* pszPattern);
    virtual void SetNumRenderLine(int iLine);
    FILTERLIST_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);
};

class CUIUnmixgemList : public CUITextListBox<UNMIX_TEXT>
{
public:
    CUIUnmixgemList();
    virtual ~CUIUnmixgemList() {}
    virtual void SetNumRenderLine(int iLine);
    UNMIX_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
    virtual void AddText(int iIndex, BYTE cComType);
    void Sort();

    inline bool IsNotified() { return m_bNotify; }
    inline bool IsEmpty() { return m_TextList.empty(); }

protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);

    bool	m_bNotify;
};

class CUIBCDeclareGuildListBox : public CUITextListBox<BCDECLAREGUILD_TEXT>

{
public:
    CUIBCDeclareGuildListBox();
    virtual ~CUIBCDeclareGuildListBox() {}

    virtual void AddText(const wchar_t* szGuildName, int nMarkCount, BYTE byIsGiveUp, BYTE bySeqNum);
    virtual void DeleteText(DWORD dwGuildIndex);
    virtual void SetNumRenderLine(int iLine);
    BCDECLAREGUILD_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
    void Sort();

protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);
};

class CUIBCGuildListBox : public CUITextListBox<BCGUILD_TEXT>

{
public:
    int		Select_Guild;
    CUIBCGuildListBox();
    virtual ~CUIBCGuildListBox() {}

    virtual void AddText(const wchar_t* szGuildName, BYTE byJoinSide, BYTE byGuildInvolved, int iGuildScore);
    virtual void DeleteText(DWORD dwGuildIndex);
    virtual void SetNumRenderLine(int iLine);
    BCGUILD_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);
};

class CUIMoveCommandListBox : public CUITextListBox<MOVECOMMAND_TEXT>
{
public:
    CUIMoveCommandListBox();
    virtual ~CUIMoveCommandListBox() {}

    virtual void AddText(int iIndex, const wchar_t* szMapName, const wchar_t* szSubMapName, int iReqLevel, int iReqZen, int iGateNum);
    //virtual void DeleteText(DWORD dwGuildIndex);
    virtual void SetNumRenderLine(int iLine);
    MOVECOMMAND_TEXT* GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
    //void Sort();

protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int iLineNumber);
    virtual BOOL DoLineMouseAction(int iLineNumber);
    virtual int GetRenderLinePos_y(int iLineNumber);
};

struct SCurQuestItem
{
    BOOL	m_bIsSelected;
    DWORD	m_dwIndex;
    wchar_t	m_szText[64];
};

class CUICurQuestListBox : public CUITextListBox<SCurQuestItem>
{
public:
    CUICurQuestListBox();
    virtual ~CUICurQuestListBox() {}

    virtual void AddText(DWORD dwQuestIndex, const wchar_t* pszText);
    virtual void DeleteText(DWORD dwQuestIndex);
    virtual void SetNumRenderLine(int nLine);
    SCurQuestItem* GetSelectedText()
    {
        return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine()));
    }

protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int nLine);
    virtual BOOL DoLineMouseAction(int nLine);
    virtual int GetRenderLinePos_y(int nLine);
};

enum REQUEST_REWARD_CLASSIFY;

struct SQuestContents
{
    BOOL	m_bIsSelected;
    HFONT	m_hFont;
    DWORD	m_dwColor;
    int		m_nSort;
    wchar_t	m_szText[64];
    REQUEST_REWARD_CLASSIFY	m_eRequestReward;
    DWORD	m_dwType;
    WORD	m_wIndex;
    ITEM* m_pItem;
};

struct SRequestRewardText;

class CUIQuestContentsListBox : public CUITextListBox<SQuestContents>
{
public:
    CUIQuestContentsListBox();
    virtual ~CUIQuestContentsListBox() {}

    virtual void AddText(HFONT hFont, DWORD dwColor, int nSort, const wchar_t* pszText);
    virtual void AddText(SRequestRewardText* pRequestRewardText, int nSort);
    /*	virtual void DeleteText(DWORD dwQuestIndex);
        virtual void SetNumRenderLine(int nLine);
        SQuestContents* GetSelectedText()
        { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
    */
protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int nLine);
    virtual void RenderCoveredInterface();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual BOOL DoLineMouseAction(int nLine);
    virtual int GetRenderLinePos_y(int nLine);
};

struct TEXTCOLOR_DATA
{
    int m_iCharPos;
    int m_iStartPos;
    DWORD m_dwTextColor;
    DWORD m_dwBackColor;
};
const int MAX_COLOR_PER_LINE = 10;

struct RENDER_TEXT_DATA
{
    wchar_t m_szText[MAX_TEXT_LENGTH + 1];
    int m_iWidth;
    int m_iHeight;
    int m_iTab;
    DWORD m_dwUseCount;
    DWORD m_dwTextColor;
    DWORD m_dwBackColor;
    int m_iTextureIndex;
    int m_iTextureIndexEx;
    BOOL m_bUseTextEX;
};

class IUIRenderText
{
public:
    virtual bool Create(HDC hDC) = 0;
    virtual void Release() = 0;

    virtual HDC GetFontDC() const = 0;
    virtual BYTE* GetFontBuffer() const = 0;

    virtual DWORD GetTextColor() const = 0;
    virtual DWORD GetBgColor() const = 0;

    virtual void SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha) = 0;
    virtual void SetTextColor(DWORD dwColor) = 0;
    virtual void SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha) = 0;
    virtual void SetBgColor(DWORD dwColor) = 0;

    virtual void SetFont(HFONT hFont) = 0;

    virtual void RenderText(int iPos_x, int iPos_y, const wchar_t* pszText, int iBoxWidth = 0, int iBoxHeight = 0,
        int iSort = RT3_SORT_LEFT, OUT SIZE* lpTextSize = NULL) = 0;
};

typedef std::multimap<int, RENDER_TEXT_DATA, std::less<int> > RTMap;

class CUIRenderTextOriginal : public IUIRenderText
{
    HDC	m_hFontDC;
    HBITMAP m_hBitmap;
    BYTE* m_pFontBuffer;
    DWORD m_dwTextColor, m_dwBackColor;
public:
    CUIRenderTextOriginal();
    virtual ~CUIRenderTextOriginal();

    bool Create(HDC hDC);
    void Release();

    HDC GetFontDC() const;
    BYTE* GetFontBuffer() const;

    DWORD GetTextColor() const;
    DWORD GetBgColor() const;

    void SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha);
    void SetTextColor(DWORD dwColor);
    void SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha);
    void SetBgColor(DWORD dwColor);

    void SetFont(HFONT hFont);

    void RenderText(int iPos_x, int iPos_y, const wchar_t* pszText, int iBoxWidth = 0, int iBoxHeight = 0,
        int iSort = RT3_SORT_LEFT, OUT SIZE* lpTextSize = NULL);

protected:
    void WriteText(int iOffset, int iWidth, int iHeight);
    void UploadText(int sx, int sy, int Width, int Height);
};

class CUIRenderText
{
    CUIRenderText();

    IUIRenderText* m_pRenderText;
    int m_iRenderTextType;

public:
    virtual ~CUIRenderText();

    static CUIRenderText* GetInstance();

    bool Create(int iRenderTextType, HDC hDC);
    void Release();

    int GetRenderTextType() const;

    HDC GetFontDC() const;
    BYTE* GetFontBuffer() const;
    DWORD GetTextColor() const;
    DWORD GetBgColor() const;

    void SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha);
    void SetTextColor(DWORD dwColor);
    void SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha);
    void SetBgColor(DWORD dwColor);

    void SetFont(HFONT hFont);

    void RenderText(int iPos_x, int iPos_y, const wchar_t* pszText, int iBoxWidth = 0, int iBoxHeight = 0,
        int iSort = RT3_SORT_LEFT, OUT SIZE* lpTextSize = NULL);
};

#define g_pRenderText CUIRenderText::GetInstance()
#define g_pMultiLanguage CMultiLanguage::GetSingletonPtr()

void SaveIMEStatus();
void RestoreIMEStatus();
void CheckTextInputBoxIME(int iMode);

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
    virtual void SetMultiline(BOOL bUseFlag) { m_bUseMultiLine = bUseFlag; }

    virtual void SetTextLimit(int iLimit);
    virtual void SetTextColor(BYTE a, BYTE r, BYTE g, BYTE b) { m_dwTextColor = _ARGB(a, r, g, b); }
    virtual void SetBackColor(BYTE a, BYTE r, BYTE g, BYTE b) { m_dwBackColor = _ARGB(a, r, g, b); }
    virtual void SetSelectBackColor(BYTE a, BYTE r, BYTE g, BYTE b) { m_dwSelectBackColor = _ARGB(a, r, g, b); }
    virtual void SetText(const wchar_t* pszText);
    virtual void GetText(wchar_t* pszText, int iGetLength = MAX_TEXT_LENGTH);

    HWND GetHandle() { return m_hEditWnd; }
    HWND GetParentHandle() { return m_hParentWnd; }
    BOOL HaveFocus() { return (GetHandle() == GetFocus()); }
    BOOL UseMultiline() { return m_bUseMultiLine; }
    virtual void SetTabTarget(CUITextInputBox* pTabTarget) { m_pTabTarget = pTabTarget; }
    CUITextInputBox* GetTabTarget() { return m_pTabTarget; }

    virtual void Lock(BOOL bFlag) { m_bLock = bFlag; }
    virtual BOOL IsLocked() { return m_bLock; }
    BOOL IsPassword() { return m_bPasswordInput; }

    virtual void SetIMEPosition();
#ifdef PBG_ADD_INGAMESHOPMSGBOX
    bool GetUseScrollbar() { return m_bUseScrollbarRender; }
    void SetUseScrollbar(bool _scrollbar = TRUE) { m_bUseScrollbarRender = _scrollbar; }
#endif //PBG_ADD_INGAMESHOPMSGBOX

protected:
    virtual BOOL DoMouseAction();
    void RenderScrollbar();

    void WriteText(int iOffset, int iWidth, int iHeight);
    void UploadText(int sx, int sy, int Width, int Height);

public:
    WNDPROC m_hOldProc;
    CTimer m_caretTimer = { };

protected:
    HWND m_hParentWnd;
    HWND m_hEditWnd;
    HDC m_hMemDC;
    HBITMAP m_hBitmap;
    BYTE* m_pFontBuffer;
    bool m_bSetText = false;
    std::wstring m_sTextToSet;


    CUITextInputBox* m_pTabTarget;

    DWORD m_dwTextColor;
    DWORD m_dwBackColor;
    DWORD m_dwSelectBackColor;

    float m_fCaretWidth;
    float m_fCaretHeight;

    BOOL m_bPasswordInput;
    BOOL m_bLock;

    BOOL m_bIsReady;
    int m_iRealWindowPos_x;
    int m_iRealWindowPos_y;

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
#endif //PBG_ADD_INGAMESHOPMSGBOX
};

class CUIChatInputBox
{
public:
    CUIChatInputBox() {}
    virtual ~CUIChatInputBox() { RemoveHistory(TRUE); }

    virtual void Init(HWND hWnd);
    void Reset();
    void Render();
    void TabMove(int iBoxNumber);
    void GetTexts(wchar_t* pText, wchar_t* pBuddyText);
    void ClearTexts();
    void SetText(BOOL bSetText, const wchar_t* pText, BOOL bSetBuddyText, const wchar_t* pBuddyText);
    void SetState(int iState);
    int GetState() { return m_TextInputBox.GetState(); }
    void SetFont(HFONT hFont);
    void SetTextPosition(int iPos_x, int iPos_y) { m_TextInputBox.SetPosition(iPos_x, iPos_y); }
    void SetBuddyPosition(int iPos_x, int iPos_y) { m_BuddyInputBox.SetPosition(iPos_x, iPos_y); }
    BOOL HaveFocus() { return (m_TextInputBox.HaveFocus() || m_BuddyInputBox.HaveFocus()); }
    BOOL DoMouseAction();
    void RestoreFocus() { m_bFocusLose = TRUE; }

protected:
    CUITextInputBox m_TextInputBox;
    CUITextInputBox m_BuddyInputBox;
    BOOL m_bFocusLose;
    int m_iBackupFocus;

public:
    virtual void AddHistory(const wchar_t* pszText);
    virtual void MoveHistory(int iDegree);
private:
    void RemoveHistory(BOOL bClear);
    BOOL m_bHistoryMode;
    wchar_t m_szTempText[MAX_TEXT_LENGTH + 1];
    std::deque<wchar_t*> m_HistoryList;
    std::deque<wchar_t*>::iterator m_CurrentHistoryLine;
    std::deque<wchar_t*>::iterator m_HistoryListIter;
};

class CUILoginInputBox : public CUIChatInputBox
{
public:
    CUILoginInputBox() {}
    virtual ~CUILoginInputBox() {}
    virtual void Init(HWND hWnd);
    virtual void AddHistory(const wchar_t* pszText) {}
    virtual void MoveHistory(int iDegree) {}
};

class CUIMercenaryInputBox : public CUIChatInputBox
{
public:
    CUIMercenaryInputBox() {}
    virtual ~CUIMercenaryInputBox() {}
    virtual void Init(HWND hWnd);
    virtual void AddHistory(const wchar_t* pszText) {}
    virtual void MoveHistory(int iDegree) {}
};

#define SLIDE_LEVEL_MAX 5
#define SLIDE_TEXT_LENGTH 1024

struct SLIDEHELPTEXT
{
    int iLevel;
    int iNumber;
    char szSlideHelpText[32][256];
};

struct SLIDEHELP
{
    int iCreateDelay;
    float fSpeed;
    SLIDEHELPTEXT SlideHelp[SLIDE_LEVEL_MAX];
};

struct SLIDE_QUEUE_DATA
{
    int m_iType;
    wchar_t* m_pszText;
    DWORD m_dwTextColor;
    float m_fSpeed;
    BOOL m_bLastData;
};

typedef std::multimap<DWORD, SLIDE_QUEUE_DATA, std::less<DWORD> > SLIDE_QUEUE;

class CUISlideHelp : public CUIControl
{
public:
    CUISlideHelp();
    virtual ~CUISlideHelp();

    void Init(BOOL bBold = FALSE, BOOL bBlink = FALSE);
    BOOL DoMouseAction();
    void Render(BOOL bForceFadeOut = FALSE);
    void SetScrollSpeed(float fSpeed);
    BOOL AddSlideText(const wchar_t* pszNewText, DWORD dwTextColor = (255 << 24) + (200 << 16) + (220 << 8) + (230));

    int GetAlphaRate() { return m_iAlphaRate; }
    BOOL HaveText();

    void AddSlide(int iLoopCount, int iLoopDelay, const wchar_t* pszText, int iType, float fSpeed, DWORD dwTextColor);
    void CheckTime();
    void ManageSlide();
    DWORD m_dwTimer;
    DWORD m_dwCurrentSecond;

    SLIDE_QUEUE m_SlideQueue;
    SLIDE_QUEUE::iterator m_SlideQueueIter;
    std::list <DWORD> m_RemoveQueueList;

protected:
    void SlideMove();
    void ComputeSpeed();

    int CheckCutSize(const wchar_t* pszSource, int iNeedValue);

protected:
    HFONT m_hFont;
    wchar_t* m_pszSlideText;
    wchar_t m_szSlideTextA[SLIDE_TEXT_LENGTH];
    wchar_t m_szSlideTextB[SLIDE_TEXT_LENGTH];
    float m_fMovePosition;
    float m_fMoveAccel;
    float m_fMoveSpeed;
    float m_fMaxMoveSpeed;
    float m_iAlphaRate;
    int m_iCutLength;
    int m_iCutSize;
    int m_iFontHeight;
    DWORD m_dwSlideTextColor;
    BOOL m_bBlink;
};

class CSlideHelpMgr
{
public:
    CSlideHelpMgr();
    virtual ~CSlideHelpMgr();

    void Init();
    void Render();

    void CreateSlideText();
    void OpenSlideTextFile(const wchar_t* szFileName);
    void ClearSlideText();
    const wchar_t* GetSlideText(int iLevel);
    void SetCreateDelay(int iDelay) { m_iCreateDelay = iDelay; }

    void AddSlide(int iLoopCount, int iLoopDelay, const wchar_t* pszText,
        int iType, float fSpeed, DWORD dwTextColor = (255 << 24) + (200 << 16) + (220 << 8) + (230));
    void ManageSlide();
    BOOL IsIdle();
protected:
    CUISlideHelp m_HelpSlide;
    CUISlideHelp m_NoticeSlide;

    int m_iCreateDelay;
    int m_iLevelCap[SLIDE_LEVEL_MAX];
    int m_iTextNumber[SLIDE_LEVEL_MAX];
    std::list<wchar_t*> m_SlideTextList[SLIDE_LEVEL_MAX];
    std::list<wchar_t*>::iterator m_SlideTextListIter;
    float m_fHelpSlideSpeed;
};

extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;

#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP
typedef struct
{
    BOOL	m_bIsSelected;

    int		m_iStorageSeq;
    int		m_iStorageItemSeq;
    int		m_iStorageGroupCode;
    int		m_iProductSeq;
    int		m_iPriceSeq;
    int		m_iCashPoint;
    int		m_iNum;
    WORD	m_wItemCode;

    wchar_t m_szName[MAX_TEXT_LENGTH];
    wchar_t m_szNum[MAX_TEXT_LENGTH];
    wchar_t m_szPeriod[MAX_TEXT_LENGTH];
    wchar_t m_szSendUserName[MAX_ID_SIZE + 1];
    wchar_t m_szMessage[MAX_GIFT_MESSAGE_SIZE];
    wchar_t m_szType;
}IGS_StorageItem;

class CUIInGameShopListBox : public CUITextListBox<IGS_StorageItem>
{
    enum IMAGE_LISTBOX_SIZE
    {
        LISTBOX_WIDTH = 146,
        LISTBOX_HEIGHT = 115,
    };
public:
    CUIInGameShopListBox();
    virtual ~CUIInGameShopListBox() {}

    virtual void AddText(IGS_StorageItem& _StorageItem);
    virtual void SetNumRenderLine(int nLine);
    IGS_StorageItem* GetSelectedText()
    {
        return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine()));
    }

protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int nLine);
    virtual BOOL DoLineMouseAction(int nLine);
    virtual int GetRenderLinePos_y(int nLine);
};

#define LINE_TEXTMAX	64
#define INFO_LINEMAX	10
#define INFO_LINE_CNTMAX	50

struct IGS_BuyList
{
    BOOL	m_bIsSelected;
    wchar_t	m_pszItemExplanation[LINE_TEXTMAX];
};

class CUIBuyingListBox : public CUITextListBox<IGS_BuyList>
{
    enum IMAGE_LISTBOX_SIZE
    {
        LISTBOX_WIDTH = 175,
        LISTBOX_HEIGHT = 95,
    };
public:
    CUIBuyingListBox();
    virtual ~CUIBuyingListBox() {}

    virtual void AddText(const wchar_t* pszExplanationText);
    virtual void SetNumRenderLine(int nLine);
    IGS_BuyList* GetSelectedText()
    {
        return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine()));
    }

    void SetLineColorRender(const bool _LineColor = true);
    const bool& GetLineColorRender() const { return m_bRenderLineColor; }

protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int nLine);
    virtual BOOL DoLineMouseAction(int nLine);
    virtual int GetRenderLinePos_y(int nLine);
    bool m_bRenderLineColor;
};

class CRadioButton
{
    enum IMAGE_RADIOBUTTON
    {
        IMAGE_CHECKBTN = BITMAP_IGS_CHECK_BUTTON,
    };
    enum SIZE_BTN
    {
        BTN_WIDTH = 14,
        BTN_HEIGHT = 14,
        BTN_SPACE = 1,
    };
    enum STATE_BUTTON
    {
        LBTN_DEFAULT = 0,
        LBTN_UP,
        LBTN_DOWN,
    };
public:
    CRadioButton();
    ~CRadioButton();
    bool UpdateMouseAction();
    bool UpdateActionCheck(int _nState);
    void RadiobuttonBoxRender();
    void SetRadioBtnRect(float _x = 0, float _y = 0, float _width = BTN_WIDTH, float _height = BTN_HEIGHT);
    bool IsRadioBtn(RECT _rt);
    void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv);
    const bool& GetCheckBtn() const { return m_bCheckState; }
    static int m_nIterIndex;
    int m_nRadioBtnEnable;
    void SetCheckState(bool _Value);
    void SetRadioBtnIsEnable(int _Value);
    int GetRadioBtnIsEnable() { return m_nRadioBtnEnable; }
private:
    RECT m_rtCheckBtn;
    bool m_bCheckState;
    BYTE m_byMouseState;
};

struct IGS_PackBuyList
{
    BOOL	m_bIsSelected;
    wchar_t	m_pszItemName[32];
    wchar_t	m_nItemInfo[32];
    CRadioButton m_CheckBtn;
};

typedef struct
{
    BOOL	m_bIsSelected;
    int		m_iPackageSeq;
    int		m_iDisplaySeq;
    int		m_iPriceSeq;
    WORD	m_wItemCode;
    int		m_iCashType;

    wchar_t m_szItemName[MAX_TEXT_LENGTH];
    wchar_t m_szItemPrice[MAX_TEXT_LENGTH];
    wchar_t m_szItemPeriod[MAX_TEXT_LENGTH];
    wchar_t m_szAttribute[MAX_TEXT_LENGTH];

    CRadioButton m_RadioBtn;
}IGS_SelectBuyItem;

class CUIPackCheckBuyingListBox : public CUITextListBox<IGS_SelectBuyItem>
{
    enum IMAGE_LISTBOX_SIZE
    {
        LISTBOX_WIDTH = 180,
        LISTBOX_HEIGHT = 100,
        TEXT_HEIGHTSIZE = 33,
    };
public:
    CUIPackCheckBuyingListBox();
    virtual ~CUIPackCheckBuyingListBox();

    virtual void AddText(IGS_SelectBuyItem& Item);
    virtual void SetNumRenderLine(int iLine);
    IGS_SelectBuyItem* GetSelectedText()
    {
        return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine()));
    }
    BOOL IsChangeLine();

protected:
    virtual void RenderInterface();
    virtual BOOL RenderDataLine(int nLine);
    virtual BOOL DoLineMouseAction(int nLine);
    virtual int GetRenderLinePos_y(int nLine);

private:
    int	m_iCurrentLine;
};
#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP

#endif	//__UICONTROL_H__
