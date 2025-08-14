#pragma once

#include "UIControls.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "WSclient.h"
#include "Dotnet/Connection.h"

#define WM_CHATROOMMSG_BEGIN (WM_USER+0x100)
#define WM_CHATROOMMSG_END (WM_USER+0x200)

const int g_ciWindowFrameThickness = 5;
const int g_ciWindowTitleHeight = 21;
const int UIWND_DEFAULT = -1;

const DWORD g_cdwLetterCost = 1000;

enum UIWINDOWSTYLE
{
    UIWINDOWSTYLE_NULL = 0, UIWINDOWSTYLE_TITLEBAR = 1, UIWINDOWSTYLE_FRAME = 2,
    UIWINDOWSTYLE_RESIZEABLE = 4, UIWINDOWSTYLE_MOVEABLE = 8, UIWINDOWSTYLE_MINBUTTON = 16, UIWINDOWSTYLE_MAXBUTTON = 32,
    UIWINDOWSTYLE_NORMAL = UIWINDOWSTYLE_TITLEBAR | UIWINDOWSTYLE_FRAME | UIWINDOWSTYLE_RESIZEABLE | UIWINDOWSTYLE_MOVEABLE | UIWINDOWSTYLE_MINBUTTON | UIWINDOWSTYLE_MAXBUTTON,
    UIWINDOWSTYLE_FIXED = UIWINDOWSTYLE_TITLEBAR | UIWINDOWSTYLE_FRAME | UIWINDOWSTYLE_MOVEABLE
};

enum UIWINDOWSTYPE
{
    UIWNDTYPE_EMPTY = 0, UIWNDTYPE_CHAT, UIWNDTYPE_CHAT_READY, UIWNDTYPE_FRIENDMAIN, UIWNDTYPE_TEXTINPUT, UIWNDTYPE_QUESTION,
    UIWNDTYPE_READLETTER, UIWNDTYPE_WRITELETTER, UIWNDTYPE_OK, UIWNDTYPE_QUESTION_FORCE, UIWNDTYPE_OK_FORCE
};

enum UIADDWINDOWOPTION
{
    UIADDWND_NULL = 0, UIADDWND_FORCEPOSITION = 1
};

const int UIPHOTOVIEWER_CANCONTROL = 1;

class CUIBaseWindow : public CUIControl
{
public:
    CUIBaseWindow();
    virtual ~CUIBaseWindow();

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    
    virtual void Refresh() {}
    void SetLimitSize(int iMinWidth, int iMinHeight, int iMaxWidth = 0, int iMaxHeight = 0)
    {
        m_iMinWidth = iMinWidth; m_iMinHeight = iMinHeight; m_iMaxWidth = iMaxWidth; m_iMaxHeight = iMaxHeight;
    }
    virtual void SetTitle(const wchar_t* pszTitle);
    const wchar_t* GetTitle() { return m_strTitle.c_str(); }
    virtual void Maximize();

    void Render();
    virtual int RPos_x(int iPos_x) { return iPos_x + (m_iPos_x + g_ciWindowFrameThickness); }
    virtual int RPos_y(int iPos_y) { return iPos_y + (m_iPos_y + g_ciWindowTitleHeight); }
    virtual int RWidth() { return m_iWidth - g_ciWindowFrameThickness * 2; }
    virtual int RHeight() { return m_iHeight - (g_ciWindowTitleHeight + g_ciWindowFrameThickness); }
    BOOL HaveTextBox() { return m_bHaveTextBox; }
    void GetBackPosition(BOOL* pbIsMaximize, int* piBackPos_y, int* piBackHeight)
    {
        *pbIsMaximize = m_bIsMaximize; *piBackPos_y = m_iBackPos_y; *piBackHeight = m_iBackHeight;
    }
    void SetBackPosition(BOOL bIsMaximize, int iBackPos_y, int iBackHeight)
    {
        m_bIsMaximize = bIsMaximize; m_iBackPos_y = iBackPos_y; m_iBackHeight = iBackHeight;
    }

    virtual BOOL CloseCheck() { return TRUE; }

protected:
    BOOL DoMouseAction();

    virtual void InitControls() = 0;

    virtual void RenderSub() {}
    virtual void RenderOver() {}
    virtual void DoActionSub(BOOL bMessageOnly) {			}
    virtual void DoMouseActionSub() {}
    void DrawOutLine(int iPos_x, int iPos_y, int iWidth, int iHeight);
    void SetControlButtonColor(int iSelect);

protected:
    int m_iMouseClickPos_x, m_iMouseClickPos_y;
    int m_iResizeDir;
    int m_iMinWidth, m_iMinHeight;
    int m_iMaxWidth, m_iMaxHeight;
    std::wstring m_strTitle;
    BOOL m_bHaveTextBox;
    int m_iControlButtonClick;
    BOOL m_bIsMaximize;
    int m_iBackPos_y, m_iBackHeight;

    std::once_flag _controlsInitialized;
};

class CUIDefaultWindow : public CUIBaseWindow
{
public:
    CUIDefaultWindow() {}
    virtual ~CUIDefaultWindow() {}

    virtual int RPos_x(int iPos_x) { return iPos_x + (m_iPos_x); }
    virtual int RPos_y(int iPos_y) { return iPos_y + (m_iPos_y); }
    virtual int RWidth() { return m_iWidth; }
    virtual int RHeight() { return m_iHeight; }
protected:
    virtual void InitControls() {}
};

class CUIChatWindow : public CUIBaseWindow
{
    static void HandlePacketS(int32_t handle, const BYTE* ReceiveBuffer, int32_t Size);
    inline static std::map<int32_t, DWORD> ConnectionHandleToWindowUuid = { };
    Connection* _connection;

public:
    CUIChatWindow();
    virtual ~CUIChatWindow();

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    virtual void Refresh();
    void FocusReset() { m_TextInputBox.GiveFocus(); }
    int AddChatPal(const wchar_t* pszID, BYTE Number, BYTE Server);
    void RemoveChatPal(const wchar_t* pszID);
    void AddChatText(BYTE byIndex, const wchar_t* pszText, int iType, int iColor);
    void ConnectToChatServer(const wchar_t* pszIP, DWORD dwRoomNumber, DWORD dwTicket);
    void DisconnectToChatServer();
    Connection* GetCurrentSocket() { return _connection; }
    GUILDLIST_TEXT* GetCurrentInvitePal() { return m_InvitePalListBox.GetSelectedText(); }
    void UpdateInvitePalList();
    int GetShowType() { return m_iShowType; }
    const wchar_t* GetChatFriend(int* piResult = NULL);
    int GetUserCount() { return m_PalListBox.GetLineNum(); }
    DWORD GetRoomNumber() { return m_dwRoomNumber; }
    void Lock(BOOL bFlag);

protected:
    virtual void RenderSub();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual void DoMouseActionSub();
    void InitControls() override;
protected:
    int m_iShowType;
    CUITextInputBox m_TextInputBox;
    CUISimpleChatListBox m_ChatListBox;
    CUIChatPalListBox m_PalListBox;
    CUIButton m_InviteButton;
    CUIChatPalListBox m_InvitePalListBox;
    CUIButton m_CloseInviteButton;
    DWORD m_dwRoomNumber;
    int m_iPrevWidth;
    wchar_t m_szLastText[MAX_CHATROOM_TEXT_LENGTH] {};
};

class CUIPhotoViewer : public CUIControl
{
public:
    CUIPhotoViewer();
    virtual ~CUIPhotoViewer();

    virtual CHARACTER* GetPhotoChar() { return &m_PhotoChar; }

    virtual void Init(int iInitType);
    virtual void SetClass(CLASS_TYPE byClass);
    virtual void SetEquipmentPacket(BYTE* pbyEquip);
    virtual void CopyPlayer();
    virtual void SetAngle(float fDegree);
    virtual void SetZoom(float fZoom);
    virtual void SetAutoupdatePlayer(BOOL bFlag) { m_bUpdatePlayer = bFlag; }

    virtual void SetAnimation(int iAnimationType);
    virtual void ChangeAnimation(int iMoveDir = 0);

    void SetID(const wchar_t* pszID);
    const wchar_t* GetID() { return m_PhotoChar.ID; }
    float GetCurrentAngle() { return m_fCurrentAngle; }
    int GetCurrentAction() { return m_iSettingAnimation; }
    float GetCurrentZoom() { return m_fCurrentZoom; }
    void SetWebzenMail(BOOL bFlag) { m_bIsWebzenMail = bFlag; }

    virtual BOOL DoMouseAction();
    virtual void Render();
protected:
    void RenderPhotoCharacter();
    int SetPhotoPose(int iCurrentAni, int iMoveDir = 0);

protected:
    CHARACTER m_PhotoChar;
    OBJECT m_PhotoHelper;
    float m_fPhotoHelperScale;
    BOOL m_bIsInitialized;
    BOOL m_bHelpEnable;
    BOOL m_bUpdatePlayer;
    BOOL m_bActionRepeatCheck;
    int m_iShowType;
    int m_iCurrentAnimation;
    int m_iSettingAnimation;
    int m_iCurrentFrame;
    float m_fSettingAngle;
    float m_fCurrentAngle;
    float m_fRotateClickPos_x;
    float m_fSettingZoom;
    float m_fCurrentZoom;
    BOOL m_bIsWebzenMail;

public:
    void    SetShowType(int Stype) { m_iShowType = Stype; }
};

class CUILetterReadWindow : public CUIBaseWindow
{
public:
    CUILetterReadWindow() :m_iShowType(2) {}
    virtual ~CUILetterReadWindow();

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    virtual void InitControls() {}
    virtual void Refresh();
    void SetLetter(LETTERLIST_TEXT* pLetterHead, const wchar_t* pLetterText);

protected:
    virtual void RenderSub();
    virtual void RenderOver();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual void DoMouseActionSub();

public:
    CUIPhotoViewer m_Photo;

protected:
    int m_iShowType;
    LETTERLIST_TEXT m_LetterHead;

    CUILetterTextListBox m_LetterTextBox;
    CUIButton m_ReplyButton;
    CUIButton m_DeleteButton;
    CUIButton m_CloseButton;
    CUIButton m_PrevButton;
    CUIButton m_NextButton;
};

class CUILetterWriteWindow : public CUIBaseWindow
{
public:
    CUILetterWriteWindow() :m_iShowType(0), m_bIsSend(FALSE) {}
    virtual ~CUILetterWriteWindow() {}

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    
    virtual void Refresh();
    void SetMailtoText(const wchar_t* pszText);
    void SetMainTitleText(const wchar_t* pszText);
    void SetMailContextText(const wchar_t* pszText);

    void SetSendState(BOOL bFlag) { m_bIsSend = bFlag; }

    virtual BOOL CloseCheck();

protected:
    void InitControls() override;
    virtual void RenderSub();
    virtual void RenderOver();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual void DoMouseActionSub();

protected:
    int m_iShowType;
    CUIPhotoViewer m_Photo;
    BOOL m_bIsSend;
    int m_iLastTabIndex;

    CUITextInputBox m_MailtoInputBox;
    CUITextInputBox m_TitleInputBox;
    CUITextInputBox m_TextInputBox;
    CUIButton m_SendButton;
    CUIButton m_CloseButton;
    CUIButton m_PrevPoseButton;
    CUIButton m_NextPoseButton;
};

class CUITabWindow : public CUIBaseWindow
{
public:
    CUITabWindow() {}
    virtual ~CUITabWindow() {}

    virtual int RPos_x(int iPos_x) { return iPos_x + (m_iPos_x); }
    virtual int RPos_y(int iPos_y) { return iPos_y + (m_iPos_y); }
    virtual int RWidth() { return m_iWidth; }
    virtual int RHeight() { return m_iHeight; }

protected:
    virtual void InitControls() {}
};

class CFriendList
{
public:
    CFriendList() :m_iCurrentSortType(0) {}
    ~CFriendList() { ClearFriendList(); }
    void AddFriend(const wchar_t* pszID, BYTE Number, BYTE Server);
    void RemoveFriend(const wchar_t* pszID);
    void ClearFriendList();
    int UpdateFriendList(std::deque<GUILDLIST_TEXT>& pDestData, const wchar_t* pszID);
    void UpdateFriendState(const wchar_t* pszID, BYTE Number, BYTE Server);
    void UpdateAllFriendState(BYTE Number, BYTE Server);
    void Sort(int iType = -1);
    int GetCurrentSortType() { return m_iCurrentSortType; }
private:
    int m_iCurrentSortType;
    std::deque<GUILDLIST_TEXT> m_FriendList;
    std::deque<GUILDLIST_TEXT>::iterator m_FriendListIter;
};

class CUIFriendListTabWindow : public CUITabWindow
{
public:
    CUIFriendListTabWindow() {}
    virtual ~CUIFriendListTabWindow() {}

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    virtual void Refresh();
    const wchar_t* GetCurrentSelectedFriend(BYTE* pNumber = NULL, BYTE* pServer = NULL);
    DWORD GetKeyMoveListUIID() { return m_PalListBox.GetUIID(); }
    void RefreshPalList();

protected:
    virtual void RenderSub();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual void DoMouseActionSub();

protected:
    CUIChatPalListBox m_PalListBox;
    CUIButton m_AddFriendButton;
    CUIButton m_DelFriendButton;
    CUIButton m_TalkButton;
    CUIButton m_LetterButton;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIChatRoomListTabWindow : public CUITabWindow
{
public:
    CUIChatRoomListTabWindow() {}
    virtual ~CUIChatRoomListTabWindow() {}

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    virtual void Refresh();
    void AddWindow(DWORD dwUIID, const wchar_t* pszTitle);
    void RemoveWindow(DWORD dwUIID);
    DWORD GetCurrentSelectedWindow();
    DWORD GetKeyMoveListUIID() { return m_WindowListBox.GetUIID(); }
    void Reset() { m_WindowListBox.Clear(); }

protected:
    virtual void RenderSub();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual void DoMouseActionSub();

public:
    CUIWindowListBox m_WindowListBox;
    CUIButton m_HideAllButton;
};

class CLetterList
{
public:
    CLetterList() :m_iCurrentSortType(0) {}
    ~CLetterList() { ClearLetterList(); }
    void AddLetter(DWORD dwLetterID, const wchar_t* pszID, const wchar_t* pszText, const wchar_t* pszDate, const wchar_t* pszTime, BOOL bIsRead);
    void RemoveLetter(DWORD dwLetterID);
    void ClearLetterList();
    int UpdateLetterList(std::deque<LETTERLIST_TEXT>& pDestData, DWORD dwSelectLineNum);
    void Sort(int iType = -1);
    DWORD GetPrevLetterID(DWORD dwLetterID);
    DWORD GetNextLetterID(DWORD dwLetterID);
    LETTERLIST_TEXT* GetLetter(DWORD dwLetterID);
    void ResetLetterSelect(BOOL bFlag);
    BOOL CheckNoReadLetter();
    int GetCurrentSortType() { return m_iCurrentSortType; }

    void CacheLetterText(DWORD dwIndex, LPFS_LETTER_TEXT pLetterText);
    LPFS_LETTER_TEXT GetLetterText(DWORD dwIndex);
    void RemoveLetterTextCache(DWORD dwIndex);
    void ClearLetterTextCache();

    int GetLineNum(DWORD dwLetterID);
    int GetLetterCount() { return m_LetterList.size(); }

private:
    int m_iCurrentSortType;
    std::deque<LETTERLIST_TEXT> m_LetterList;
    std::deque<LETTERLIST_TEXT>::iterator m_LetterListIter;

    std::map<DWORD, FS_LETTER_TEXT, std::less<DWORD> > m_LetterCache;
    std::map<DWORD, FS_LETTER_TEXT, std::less<DWORD> >::iterator m_LetterCacheIter;
};

class CUILetterBoxTabWindow : public CUITabWindow
{
public:
    CUILetterBoxTabWindow() :m_bCheckAllState(FALSE) {}
    virtual ~CUILetterBoxTabWindow() {}

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    virtual void Refresh();
    LETTERLIST_TEXT* GetCurrentSelectedLetter();
    DWORD GetKeyMoveListUIID() { return m_LetterListBox.GetUIID(); }
    void RefreshLetterList();
    void CheckAll(BOOL bCheck);
    void PrevNextCursorMove(int iMove);

protected:
    virtual void RenderSub();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual void DoMouseActionSub();

protected:
    CUILetterListBox m_LetterListBox;
    CUIButton m_WriteButton;
    CUIButton m_ReadButton;
    CUIButton m_ReplyButton;
    CUIButton m_DeleteButton;
    BOOL m_bCheckAllState;
};

class CUIFriendWindow : public CUIBaseWindow
{
public:
    CUIFriendWindow() :m_iTabIndex(0), m_iTabMouseOverIndex(0) {}
    virtual ~CUIFriendWindow() {}

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    virtual void Refresh();
    void Reset();
    void Close();
    void RefreshPalList() { m_FriendListWnd.RefreshPalList(); }

    void AddWindow(DWORD dwUIID, const wchar_t* pszTitle)
    {
        m_ChatRoomListWnd.AddWindow(dwUIID, pszTitle);
    }
    void RemoveWindow(DWORD dwUIID)
    {
        m_ChatRoomListWnd.RemoveWindow(dwUIID);
    }
    DWORD GetCurrentSelectedWindow()
    {
        return m_ChatRoomListWnd.GetCurrentSelectedWindow();
    }
    void ResetWindow()
    {
        m_ChatRoomListWnd.Reset();
    }

    void RefreshLetterList() { m_LetterBoxWnd.RefreshLetterList(); }
    LETTERLIST_TEXT* GetCurrentSelectedLetter()
    {
        return m_LetterBoxWnd.GetCurrentSelectedLetter();
    }
    void PrevNextCursorMove(int iMove)
    {
        m_LetterBoxWnd.PrevNextCursorMove(iMove);
    }

    void SetTabIndex(int iIndex) { m_iTabIndex = iIndex; }
    int GetTabIndex() { return m_iTabIndex; }

protected:
    virtual void InitControls() {}
    virtual void RenderSub();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual void DoMouseActionSub();

protected:
    int m_iTabIndex;
    int m_iTabMouseOverIndex;
    CUIFriendListTabWindow m_FriendListWnd;
    CUIChatRoomListTabWindow m_ChatRoomListWnd;
    CUILetterBoxTabWindow m_LetterBoxWnd;
};

class CUITextInputWindow : public CUIBaseWindow
{
public:
    CUITextInputWindow() :m_dwReturnWindowUIID(0) {}
    virtual ~CUITextInputWindow() {}

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    virtual void Refresh();
    void SetText(const wchar_t* pszText) { m_TextInputBox.SetText(pszText); m_TextInputBox.GiveFocus(TRUE); }

protected:
    void InitControls() override;
    virtual void RenderSub();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual void DoMouseActionSub();

    void ReturnText();

protected:
    DWORD m_dwReturnWindowUIID;
    CUITextInputBox m_TextInputBox;
    CUIButton m_AddButton;
    CUIButton m_CancelButton;
};

class CUIQuestionWindow : public CUIBaseWindow
{
public:
    CUIQuestionWindow(int iDialogType = 0) :m_dwReturnWindowUIID(0), m_iDialogType(iDialogType) {}
    virtual ~CUIQuestionWindow() {}

    virtual void Init(const wchar_t* pszTitle, DWORD dwParentID = 0);
    virtual void Refresh();

    void SaveID(const wchar_t* pszText);

protected:
    virtual void InitControls() {}
    virtual void RenderSub();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);

protected:
    DWORD m_dwReturnWindowUIID;
    int m_iDialogType;	// 0: Y/N, 1: OK
    wchar_t m_szCaption[2][MAX_TEXT_LENGTH + 1];
    wchar_t m_szSaveID[MAX_ID_SIZE + 1];
    CUIButton m_AddButton;
    CUIButton m_CancelButton;
};

typedef std::map<DWORD, CUIBaseWindow*, std::less<DWORD> > WndMap;

class CUIWindowMgr : public CUIMessage
{
public:
    CUIWindowMgr();
    virtual ~CUIWindowMgr();

    void Reset();
    DWORD AddWindow(int iWindowType, int iPos_x, int iPos_y, const wchar_t* pszTitle, DWORD dwParentID = 0, int iOption = UIADDWND_NULL);
    void RemoveWindow(DWORD dwUIID);
    void Render();
    void DoAction();
    void ShowHideWindow(DWORD dwUIID, BOOL bShowWindow);
    void HideAllWindow(BOOL bHide, BOOL bMainClose = FALSE);
    void HideAllWindowClear();
    CUIBaseWindow* GetWindow(DWORD dwUIID);
    BOOL IsWindow(DWORD dwUIID);
    CUIFriendWindow* GetFriendMainWindow() { return (CUIFriendWindow*)GetWindow(m_dwMainWindowUIID); }
    void SetWindowsEnable(DWORD bWindowsEnable) { m_bWindowsEnable = bWindowsEnable; }
    BOOL GetWindowsEnable() { return m_bWindowsEnable; }
    DWORD GetTopWindowUIID() { return (m_WindowArrangeList.empty() == TRUE ? 0 : *m_WindowArrangeList.rbegin()); }
    DWORD GetTopNotMainWindowUIID();

    void AddWindowFinder(CUIBaseWindow* pWindow);
    void RemoveWindowFinder(DWORD dwUIID);
    void SendUIMessageToWindow(DWORD dwUIID, int iMessage, int iParam1, int iParam2);

    void OpenMainWnd(int iPos_x, int iPos_y);
    void CloseMainWnd();
    void RefreshMainWndPalList() { if (m_dwMainWindowUIID != 0) GetFriendMainWindow()->RefreshPalList(); }
    void RefreshMainWndLetterList() { if (m_dwMainWindowUIID != 0) GetFriendMainWindow()->RefreshLetterList(); }
    void RefreshMainWndChatRoomList();

    void SetChatReject(BOOL bChatReject) { m_bChatReject = bChatReject; }
    BOOL GetChatReject() { return m_bChatReject; }

    BOOL LetterReadCheck(DWORD dwLetterID);
    void CloseLetterRead(DWORD dwLetterID);
    void SetLetterReadWindow(DWORD dwLetterID, DWORD dwWindowUIID);
    DWORD GetLetterReadWindow(DWORD dwLetterID);

    void SetServerEnable(BOOL bFlag);
    BOOL IsServerEnable() { return m_bServerEnable; }

    void SetAddFriendWindow(DWORD dwAddWindowUIID) { m_dwAddWindowUIID = dwAddWindowUIID; }
    DWORD GetAddFriendWindow() { return m_dwAddWindowUIID; }

    void AddForceTopWindowList(DWORD dwWindowUIID);
    void RemoveForceTopWindowList(DWORD dwWindowUIID);
    BOOL IsForceTopWindow(DWORD dwWindowUIID);
    BOOL HaveForceTopWindow() { return !m_ForceTopWindowList.empty(); }

    BOOL IsRenderFrame() { return m_bRenderFrame; }

protected:
    void HandleMessage();

public:
    BOOL m_bRenderFrame;

protected:
    BOOL m_bWindowsEnable;
    DWORD m_dwMainWindowUIID;
    WndMap m_WindowMap;
    WndMap m_WindowFindMap;
    WndMap m_WindowReadyMap;
    WndMap::iterator m_WindowMapIter;
    std::list<DWORD> m_WindowArrangeList;
    std::list<DWORD>::iterator m_WindowArrangeListIter;
    std::list<DWORD>::reverse_iterator m_WindowReverseArrangeListIter;
    std::map<DWORD, DWORD, std::less<DWORD> > m_LetterReadMap;
    std::map<DWORD, DWORD, std::less<DWORD> >::iterator m_LetterReadMapIter;
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
    DWORD m_dwAddWindowUIID;
};

class CUIFriendMenu : public CUIBaseWindow
{
public:
    CUIFriendMenu() { Init(); }
    virtual ~CUIFriendMenu() { Reset(); }

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
    BOOL IsNewMailAlert() { return m_bNewMailAlert; }

    int GetBlinkTemp();
    void IncreaseBlinkTemp();
    int GetLetterBlink();
    void IncreaseLetterBlink();

    void RenderFriendButton();

    DWORD CheckChatRoomDuplication(const wchar_t* pszTargetName);
    void SendChatRoomConnectCheck();
    void UpdateAllChatWindowInviteList();

    BOOL IsHotkeyEnable() { return m_bHotKey; }

    void AddRequestWindow(const wchar_t* szTargetName);
    BOOL IsRequestWindow(const wchar_t* szTargetName);
    void RemoveRequestWindow(const wchar_t* szTargetName);
    void RemoveAllRequestWindow();

    void CloseAllChatWindow();
    void LockAllChatWindow();
protected:
    virtual void InitControls() {}
    virtual void RenderSub();
    virtual BOOL HandleMessage();
    virtual void DoActionSub(BOOL bMessageOnly);
    virtual void DoMouseActionSub();

    void RenderWindowList();
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
