#pragma once

#include "UI/Widgets/UIControls.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Network/Server/WSclient.h"
#include "Dotnet/Connection.h"
#include <memory>
#include <mutex>

#define WM_CHATROOMMSG_BEGIN (WM_USER + 0x100)
#define WM_CHATROOMMSG_END (WM_USER + 0x200)

const int g_ciWindowFrameThickness = 5;
const int g_ciWindowTitleHeight = 21;
const int UIWND_DEFAULT = -1;

const DWORD g_cdwLetterCost = 1000;

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

enum UIWINDOWSTYPE
{
    UIWNDTYPE_EMPTY = 0,
    UIWNDTYPE_CHAT,
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

const int UIPHOTOVIEWER_CANCONTROL = 1;

class FriendWindowViews;
class CUIPhotoViewer;
namespace UI::Party { class FriendShell; class ChatRoomView; class LetterReadView; class LetterWriteView; }

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
    virtual int RPos_x(int iPos_x)
    {
        return iPos_x + (m_iPos_x + g_ciWindowFrameThickness);
    }
    virtual int RPos_y(int iPos_y)
    {
        return iPos_y + (m_iPos_y + g_ciWindowTitleHeight);
    }
    virtual int RWidth()
    {
        return m_iWidth - g_ciWindowFrameThickness * 2;
    }
    virtual int RHeight()
    {
        return m_iHeight - (g_ciWindowTitleHeight + g_ciWindowFrameThickness);
    }
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

class CUIDefaultWindow : public CUIBaseWindow
{
public:
    CUIDefaultWindow() {}
    virtual ~CUIDefaultWindow() {}

    virtual int RPos_x(int iPos_x)
    {
        return iPos_x + (m_iPos_x);
    }
    virtual int RPos_y(int iPos_y)
    {
        return iPos_y + (m_iPos_y);
    }
    virtual int RWidth()
    {
        return m_iWidth;
    }
    virtual int RHeight()
    {
        return m_iHeight;
    }

protected:
    virtual void InitControls() {}
};

class CUIChatWindow : public CUIBaseWindow
{
    static void HandlePacketS(int32_t handle, const BYTE* ReceiveBuffer, int32_t Size);
    inline static std::map<int32_t, DWORD> ConnectionHandleToWindowUuid = {};
    Connection* _connection;

public:
    CUIChatWindow();
    virtual ~CUIChatWindow();

    void Init(const wchar_t* pszTitle, DWORD dwParentID = 0) override;
    void Refresh() override;
    BOOL DoAction(BOOL messageOnly = FALSE) override;
    void Maximize() override;
    void FocusReset();
    int AddChatPal(const wchar_t* pszID, BYTE Number, BYTE Server);
    void RemoveChatPal(const wchar_t* pszID);
    void AddChatText(BYTE byIndex, const wchar_t* pszText, int iType, int iColor);
    void ConnectToChatServer(const wchar_t* pszIP, DWORD dwRoomNumber, DWORD dwTicket);
    void DisconnectToChatServer();
    Connection* GetCurrentSocket()
    {
        return _connection;
    }
    // The invitation list's pick, by name; nullptr with nothing picked.
    const wchar_t* GetCurrentInvitePal();
    void UpdateInvitePalList();
    int GetShowType();
    const wchar_t* GetChatFriend(int* piResult = NULL);
    int GetUserCount();
    DWORD GetRoomNumber()
    {
        return m_dwRoomNumber;
    }
    void Lock(BOOL bFlag);
    bool HasSemanticView() const override
    {
        return true;
    }
    bool SyncSemanticView(bool shown) override;
    bool SemanticFieldHasFocus() const override;
    void PullSemanticViewToFront() override;

protected:
    void InitControls() override {}
    BOOL HandleMessage() override;

private:
    void RefreshRoomTitle();

    DWORD m_dwRoomNumber;
    std::unique_ptr<UI::Party::ChatRoomView> m_View;
};

class CUIPhotoViewer : public CUIControl
{
public:
    CUIPhotoViewer();
    virtual ~CUIPhotoViewer();

    virtual CHARACTER* GetPhotoChar()
    {
        return &m_PhotoChar;
    }

    virtual void Init(int iInitType);
    virtual void SetClass(CLASS_TYPE byClass);
    virtual void SetEquipmentPacket(BYTE* pbyEquip);
    virtual void CopyPlayer();
    virtual void SetAngle(float fDegree);
    virtual void SetZoom(float fZoom);
    virtual void SetAutoupdatePlayer(BOOL bFlag)
    {
        m_bUpdatePlayer = bFlag;
    }

    virtual void SetAnimation(int iAnimationType);
    virtual void ChangeAnimation(int iMoveDir = 0);

    void SetID(const wchar_t* pszID);
    const wchar_t* GetID()
    {
        return m_PhotoChar.ID;
    }
    float GetCurrentAngle()
    {
        return m_fCurrentAngle;
    }
    int GetCurrentAction()
    {
        return m_iSettingAnimation;
    }
    float GetCurrentZoom()
    {
        return m_fCurrentZoom;
    }
    void SetWebzenMail(BOOL bFlag)
    {
        m_bIsWebzenMail = bFlag;
    }

    virtual BOOL DoMouseAction();
    virtual void Render();

    // Driven by UI::Party::PhotoViewerControl, which owns these gestures while the viewer stands
    // behind an RmlUi document and the native press never arrives. See its header.
    void TurnBy(float degrees);
    void ResetView();
    void ToggleHelp();

protected:
    void RenderPhotoCharacter();
    void ShowHelpText();
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
    void SetShowType(int Stype)
    {
        m_iShowType = Stype;
    }
};

class CUILetterReadWindow : public CUIBaseWindow
{
public:
    CUILetterReadWindow();
    ~CUILetterReadWindow() override;

    void Init(const wchar_t* pszTitle, DWORD dwParentID = 0) override;
    void Refresh() override;
    BOOL DoAction(BOOL messageOnly = FALSE) override;
    void Maximize() override;
    void SetLetter(LETTERLIST_TEXT* pLetterHead, const wchar_t* pLetterText);
    // The three button actions the view hands back.
    void Reply();
    void AskDelete();
    // -1 for the previous letter, +1 for the next; native had two copies of these twenty lines.
    void StepLetter(int direction);
    bool HasSemanticView() const override
    {
        return true;
    }
    bool SyncSemanticView(bool shown) override;
    void PullSemanticViewToFront() override;


    // Drawn in the post-RmlUi seam, not RenderOver(): the panel would cover it otherwise.
    void RenderAboveRmlUi() override;

protected:
    void InitControls() override {}
    BOOL HandleMessage() override;

public:
    CUIPhotoViewer m_Photo;

private:
    LETTERLIST_TEXT m_LetterHead;
    std::unique_ptr<UI::Party::LetterReadView> m_View;
};

class CUILetterWriteWindow : public CUIBaseWindow
{
public:
    CUILetterWriteWindow();
    ~CUILetterWriteWindow() override;

    void Init(const wchar_t* pszTitle, DWORD dwParentID = 0) override;
    void Refresh() override;
    BOOL DoAction(BOOL messageOnly = FALSE) override;
    void Maximize() override;
    void SetMailtoText(const wchar_t* pszText);
    void SetMainTitleText(const wchar_t* pszText);
    void SetMailContextText(const wchar_t* pszText);
    void SetSendState(BOOL bFlag);
    // The two button actions the view hands back.
    void Send();
    void RequestClose();

    BOOL CloseCheck() override;
    bool HasSemanticView() const override
    {
        return true;
    }
    bool SyncSemanticView(bool shown) override;
    bool SemanticFieldHasFocus() const override;
    void PullSemanticViewToFront() override;


    // Drawn in the post-RmlUi seam, not RenderOver(): the panel would cover it otherwise.
    void RenderAboveRmlUi() override;

protected:
    void InitControls() override {}
    BOOL HandleMessage() override;

public:
    CUIPhotoViewer m_Photo;

private:
    BOOL m_bIsSend;
    std::unique_ptr<UI::Party::LetterWriteView> m_View;
};

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

    void CacheLetterText(DWORD dwIndex, LPFS_LETTER_TEXT pLetterText);
    LPFS_LETTER_TEXT GetLetterText(DWORD dwIndex);
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

    std::map<DWORD, FS_LETTER_TEXT, std::less<DWORD>> m_LetterCache;
    std::map<DWORD, FS_LETTER_TEXT, std::less<DWORD>>::iterator m_LetterCacheIter;
};

class CUIFriendWindow : public CUIBaseWindow
{
public:
    CUIFriendWindow();
    ~CUIFriendWindow() override;
    void Init(const wchar_t* title, DWORD parent = 0) override;
    void Refresh() override;
    BOOL DoAction(BOOL messageOnly = FALSE) override;
    void Maximize() override;
    void Reset();
    void Close();
    void RefreshPalList();
    void RefreshLetterList();
    void AddWindow(DWORD id, const wchar_t* title);
    void RemoveWindow(DWORD id);
    void ResetWindow();
    DWORD GetCurrentSelectedWindow();
    LETTERLIST_TEXT* GetCurrentSelectedLetter();
    void PrevNextCursorMove(int line);
    void SetTabIndex(int tab);
    int GetTabIndex();
    bool HasSemanticView() const override { return true; }
    bool SyncSemanticView(bool shown) override;
    void PullSemanticViewToFront() override;
    void RestoreSemanticLayout(int x, int y, int width, int height);
    void RestoreSemanticMaximized();

protected:
    void InitControls() override {}
    BOOL HandleMessage() override;

private:
    float SemanticScaleRatio() const;

    std::unique_ptr<UI::Party::FriendShell> m_Shell;
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
    // The family's native 3D, drawn after RmlUi has composited -- for the front window only.
    void RenderOverlay3D();
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

    void AddWindowFinder(CUIBaseWindow* pWindow);
    void RemoveWindowFinder(DWORD dwUIID);
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
    WndMap m_WindowFindMap;
    WndMap m_WindowReadyMap;
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

    void RenderFriendButton();

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
