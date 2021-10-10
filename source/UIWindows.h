#ifndef __UIWINDOW_H__
#define __UIWINDOW_H__

#pragma once

#include "UIControls.h"
#include "wsctlc.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "WSclient.h"
// WM_USER+0x100 부터 0x100개를 채팅 윈도우 소켓용으로 사용합니다.
#define WM_CHATROOMMSG_BEGIN (WM_USER+0x100)
#define WM_CHATROOMMSG_END (WM_USER+0x200)

#define CHATCONNECT_TIMER 1002
#ifdef TEENAGER_REGULATION
#define WARNING_TIMER	1004
#endif

const int g_ciWindowFrameThickness = 5;			// 윈도우 프레임 두께
const int g_ciWindowTitleHeight = 21;			// 윈도우 타이틀 높이
const int UIWND_DEFAULT = -1;					// 윈도우 기본 위치로 생성

const DWORD g_cdwLetterCost = 1000;				// 편지 발송 비용

enum UIWINDOWSTYLE	// 윈도우 스타일
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

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh() {}
	void SetLimitSize(int iMinWidth, int iMinHeight, int iMaxWidth = 0, int iMaxHeight = 0)
		{ m_iMinWidth = iMinWidth; m_iMinHeight = iMinHeight; m_iMaxWidth = iMaxWidth; m_iMaxHeight = iMaxHeight; }
	virtual void SetTitle(const char* pszTitle);
	const char* GetTitle() { return m_pszTitle; }
	virtual void Maximize();

	void Render();						// 기본 틀을 그린다
	virtual int RPos_x(int iPos_x) { return iPos_x + (m_iPos_x + g_ciWindowFrameThickness); }
	virtual int RPos_y(int iPos_y) { return iPos_y + (m_iPos_y + g_ciWindowTitleHeight); }
	virtual int RWidth() { return m_iWidth - g_ciWindowFrameThickness * 2; }
	virtual int RHeight() { return m_iHeight - (g_ciWindowTitleHeight + g_ciWindowFrameThickness); }
	BOOL HaveTextBox() { return m_bHaveTextBox; }
	void GetBackPosition(BOOL * pbIsMaximize, int * piBackPos_y, int * piBackHeight)
	{ *pbIsMaximize = m_bIsMaximize; *piBackPos_y = m_iBackPos_y; *piBackHeight = m_iBackHeight; }
	void SetBackPosition(BOOL bIsMaximize, int iBackPos_y, int iBackHeight)
	{ m_bIsMaximize = bIsMaximize; m_iBackPos_y = iBackPos_y; m_iBackHeight = iBackHeight; }

	virtual BOOL CloseCheck() { return TRUE; }			// 닫아도 되는가 체크 (TRUE는 닫아도 된다)
	
protected:
	BOOL DoMouseAction();				// 기본 틀의 마우스의 액션을 처리한다.
	// 윈도우 내부의 범위를 보내준다

	virtual void RenderSub() {}			// 하위 요소들을 그린다
	virtual void RenderOver() {}		// 윈도우 위에 그릴 것들
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DoActionSub()					{ return;	}	// 하위 요소들의 메시지, 마우스 액션 등을 처리한다.
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DoActionSub(BOOL bMessageOnly)	{			}	// 하위 요소들의 메시지, 마우스 액션 등을 처리한다.
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DoMouseActionSub() {}	// 하위 요소들의 마우스 액션을 처리한다.
	void DrawOutLine(int iPos_x, int iPos_y, int iWidth, int iHeight);
	void SetControlButtonColor(int iSelect);

protected:
	int m_iMouseClickPos_x, m_iMouseClickPos_y;
	int m_iResizeDir;	// 크기조절 방향
	int m_iMinWidth, m_iMinHeight;		// 크기 제한
	int m_iMaxWidth, m_iMaxHeight;
	char* m_pszTitle;				// 윈도우 이름
	BOOL m_bHaveTextBox;			// 한개 이상의 텍스트 박스가 있나
	int m_iControlButtonClick;		// 최소(1) 최대(2) 닫기(3) 버튼 선택
	BOOL m_bIsMaximize;				// 최대화 상태인가
	int m_iBackPos_y, m_iBackHeight;

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	const int MAX_LETTER_TITLE_LENGTH_UTF16, MAX_LETTER_TEXT_LENGTH_UTF16;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIChatWindow : public CUIBaseWindow
{
public:
	CUIChatWindow();
	virtual ~CUIChatWindow();

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh();
	void FocusReset() { m_TextInputBox.GiveFocus(); }
	int AddChatPal(const char* pszID, BYTE Number, BYTE Server);
	void RemoveChatPal(const char* pszID);
	void AddChatText(BYTE byIndex, const char* pszText, int iType, int iColor);
	void ConnectToChatServer(const char* pszIP, DWORD dwRoomNumber, DWORD dwTicket);
	void DisconnectToChatServer();
	CWsctlc * GetCurrentSocket();
	GUILDLIST_TEXT * GetCurrentInvitePal() { return m_InvitePalListBox.GetSelectedText(); }
	void UpdateInvitePalList();
	int GetShowType() { return m_iShowType; }
	const char* GetChatFriend(int * piResult = NULL);
	int GetUserCount() { return m_PalListBox.GetLineNum(); }
	DWORD GetRoomNumber() { return m_dwRoomNumber; }
	void Lock(BOOL bFlag);

protected:
	virtual void RenderSub();			// 하위 요소들을 그린다
	virtual BOOL HandleMessage();		// 메시지 처리 함수
	virtual void DoActionSub(BOOL bMessageOnly);			// 하위 요소들의 메시지, 마우스 액션 등을 처리한다.
	virtual void DoMouseActionSub();	// 하위 요소들의 마우스 액션을 처리한다.

protected:
	int m_iShowType;	// 창 표시 타입 (0: 보통, 1: 대화상대 표시, 2: 친구초대 표시)
	CUITextInputBox m_TextInputBox;
	CUISimpleChatListBox m_ChatListBox;
	CUIChatPalListBox m_PalListBox;
	CUIButton m_InviteButton;
	CUIChatPalListBox m_InvitePalListBox;
	CUIButton m_CloseInviteButton;
	DWORD m_dwRoomNumber;	// 방 번호
	int m_iPrevWidth;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	const int MAX_CHATROOM_TEXT_LENGTH_UTF16;
	char m_szLastText[MAX_CHATROOM_TEXT_LENGTH];
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	char m_szLastText[MAX_TEXT_LENGTH + 1];
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIPhotoViewer : public CUIControl
{
public:
	CUIPhotoViewer();
	virtual ~CUIPhotoViewer();

	virtual CHARACTER * GetPhotoChar() { return &m_PhotoChar; }

	virtual void Init(int iInitType);
	virtual void SetClass(BYTE byClass);
	virtual void SetEquipmentPacket(BYTE * pbyEquip);
	virtual void CopyPlayer();
	virtual void SetAngle(float fDegree);
	virtual void SetZoom(float fZoom);
	virtual void SetAutoupdatePlayer(BOOL bFlag) { m_bUpdatePlayer = bFlag; }

	virtual void SetAnimation(int iAnimationType);
	virtual void ChangeAnimation(int iMoveDir = 0);

	void SetID(const char* pszID);
	const char* GetID() { return m_PhotoChar.ID; }
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

public :
    void    SetShowType ( int Stype ) { m_iShowType = Stype; }
};

class CUILetterReadWindow : public CUIBaseWindow
{
public:
	CUILetterReadWindow():m_iShowType(2) {}
	virtual ~CUILetterReadWindow();

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh();
	void SetLetter(LETTERLIST_TEXT * pLetterHead, const char* pLetterText);

protected:
	virtual void RenderSub();			// 하위 요소들을 그린다
	virtual void RenderOver();			// 윈도우 위에 그릴 것들
	virtual BOOL HandleMessage();		// 메시지 처리 함수
	virtual void DoActionSub(BOOL bMessageOnly);			// 하위 요소들의 메시지, 마우스 액션 등을 처리한다.
	virtual void DoMouseActionSub();	// 하위 요소들의 마우스 액션을 처리한다.
	
public:
	CUIPhotoViewer m_Photo;

protected:
	int m_iShowType;	// 창 표시 타입 (0: 보통, 1: 대화상대 표시, 2: 친구초대 표시)
	LETTERLIST_TEXT m_LetterHead;

	CUILetterTextListBox m_LetterTextBox;
	CUIButton m_ReplyButton;
	CUIButton m_DeleteButton;
	CUIButton m_CloseButton;
	CUIButton m_PrevButton;
	CUIButton m_NextButton;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUILetterWriteWindow : public CUIBaseWindow
{
public:
	CUILetterWriteWindow():m_iShowType(0),m_bIsSend(FALSE) {}
	virtual ~CUILetterWriteWindow() {}

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh();
	void SetMailtoText(const char* pszText);
	void SetMainTitleText(const char* pszText);
	void SetMailContextText(const char* pszText);

	void SetSendState(BOOL bFlag) { m_bIsSend = bFlag; }

	virtual BOOL CloseCheck();			// 닫아도 되는가 체크 (TRUE는 닫아도 된다)

protected:
	virtual void RenderSub();			// 하위 요소들을 그린다
	virtual void RenderOver();			// 윈도우 위에 그릴 것들
	virtual BOOL HandleMessage();		// 메시지 처리 함수
	virtual void DoActionSub(BOOL bMessageOnly);			// 하위 요소들의 메시지, 마우스 액션 등을 처리한다.
	virtual void DoMouseActionSub();	// 하위 요소들의 마우스 액션을 처리한다.

protected:
	int m_iShowType;	// 창 표시 타입 (0: 보통, 1: 대화상대 표시, 2: 친구초대 표시)
	CUIPhotoViewer m_Photo;
	BOOL m_bIsSend;	// 보내는 중인가
	int m_iLastTabIndex;

	CUITextInputBox m_MailtoInputBox;
	CUITextInputBox m_TitleInputBox;
	CUITextInputBox m_TextInputBox;
	CUIButton m_SendButton;
	CUIButton m_CloseButton;
	//CUIButton m_PhotoShowButton;
	CUIButton m_PrevPoseButton;
	CUIButton m_NextPoseButton;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUITabWindow : public CUIBaseWindow
{
public:
	CUITabWindow() {}
	virtual ~CUITabWindow() {}

	virtual int RPos_x(int iPos_x) { return iPos_x + (m_iPos_x); }
	virtual int RPos_y(int iPos_y) { return iPos_y + (m_iPos_y); }
	virtual int RWidth() { return m_iWidth; }
	virtual int RHeight() { return m_iHeight; }
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CFriendList
{
public:
	CFriendList():m_iCurrentSortType(0) {}
	~CFriendList() { ClearFriendList(); }
	void AddFriend(const char* pszID, BYTE Number, BYTE Server);	// 친구 추가
	void RemoveFriend(const char* pszID);						// 친구 삭제
	void ClearFriendList();
	int UpdateFriendList(std::deque<GUILDLIST_TEXT> & pDestData, const char* pszID);
	void UpdateFriendState(const char* pszID, BYTE Number, BYTE Server);
	void UpdateAllFriendState(BYTE Number, BYTE Server);
	void Sort(int iType = -1);
	int GetCurrentSortType() { return m_iCurrentSortType; }
private:
	int m_iCurrentSortType;
	std::deque<GUILDLIST_TEXT> m_FriendList;
	std::deque<GUILDLIST_TEXT>::iterator m_FriendListIter;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIFriendListTabWindow : public CUITabWindow
{
public:
	CUIFriendListTabWindow() {}
	virtual ~CUIFriendListTabWindow() {}

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh();
	const char* GetCurrentSelectedFriend(BYTE * pNumber = NULL, BYTE * pServer = NULL);		// 현재 선택된 친구 ID
	DWORD GetKeyMoveListUIID() { return m_PalListBox.GetUIID(); }
	void RefreshPalList();

protected:
	virtual void RenderSub();			// 하위 요소들을 그린다
	virtual BOOL HandleMessage();		// 메시지 처리 함수
	virtual void DoActionSub(BOOL bMessageOnly);			// 하위 요소들의 메시지, 마우스 액션 등을 처리한다.
	virtual void DoMouseActionSub();	// 하위 요소들의 마우스 액션을 처리한다.

protected:
	CUIChatPalListBox m_PalListBox;
	CUIButton m_AddFriendButton;
	CUIButton m_DelFriendButton;
	CUIButton m_TalkButton;
	CUIButton m_LetterButton;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CHATROOM_SOCKET
{
	DWORD m_dwRoomID;		// 방 번호
	CWsctlc m_WSClient;
	DWORD m_dwWindowUIID;	// 윈도우 번호
};

class CChatRoomSocketList
{
public:
	CChatRoomSocketList() { ClearChatRoomSocketList(); }
	~CChatRoomSocketList() { ClearChatRoomSocketList(); }
	BOOL AddChatRoomSocket(DWORD dwRoomID, DWORD dwWindowUIID, const char* pszIP);	// 채팅 소켓 추가
	void RemoveChatRoomSocket(DWORD dwRoomID);					// 친구 삭제
	void ClearChatRoomSocketList();
	CHATROOM_SOCKET * GetChatRoomSocketData(DWORD dwRoomID);
	void ProcessSocketMessage(DWORD dwSocketID, WORD wMessage);
	void ProtocolCompile();
private:
	DWORD CreateChatRoomSocketID(DWORD dwRoomID);
	DWORD GetChatRoomSocketID(DWORD dwSocketID);

private:
	BYTE m_bCurrectCreateID;
	BYTE m_bChatRoomSocketStatus[256];
	std::map<DWORD, DWORD, std::less<DWORD> > m_ChatRoomSocketStatusMap;
	std::map<DWORD, DWORD, std::less<DWORD> >::iterator m_ChatRoomSocketStatusMapIter;

	std::map<DWORD, CHATROOM_SOCKET *, std::less<DWORD> > m_ChatRoomSocketMap;
	std::map<DWORD, CHATROOM_SOCKET *, std::less<DWORD> >::iterator m_ChatRoomSocketMapIter;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIChatRoomListTabWindow : public CUITabWindow
{
public:
	CUIChatRoomListTabWindow() {}
	virtual ~CUIChatRoomListTabWindow() {}

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh();
	void AddWindow(DWORD dwUIID, const char* pszTitle);	// 윈도우 추가
	void RemoveWindow(DWORD dwUIID);				// 윈도우 제거
	DWORD GetCurrentSelectedWindow();				// 현재 선택된 윈도우 UIID 얻기
	DWORD GetKeyMoveListUIID() { return m_WindowListBox.GetUIID(); }
	void Reset() { m_WindowListBox.Clear(); }

protected:
	virtual void RenderSub();			// 하위 요소들을 그린다
	virtual BOOL HandleMessage();		// 메시지 처리 함수
	virtual void DoActionSub(BOOL bMessageOnly);			// 하위 요소들의 메시지, 마우스 액션 등을 처리한다.
	virtual void DoMouseActionSub();	// 하위 요소들의 마우스 액션을 처리한다.

public:
	CUIWindowListBox m_WindowListBox;
	CUIButton m_HideAllButton;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CLetterList
{
public:
	CLetterList():m_iCurrentSortType(0) {}
	~CLetterList() { ClearLetterList(); }
	void AddLetter(DWORD dwLetterID, const char* pszID, const char* pszText, const char* pszDate, const char* pszTime, BOOL bIsRead);	// 편지 추가
	void RemoveLetter(DWORD dwLetterID);														// 편지 제거
	void ClearLetterList();
	int UpdateLetterList(std::deque<LETTERLIST_TEXT> & pDestData, DWORD dwSelectLineNum);
	void Sort(int iType = -1);
	DWORD GetPrevLetterID(DWORD dwLetterID);
	DWORD GetNextLetterID(DWORD dwLetterID);
	LETTERLIST_TEXT * GetLetter(DWORD dwLetterID);
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
	CUILetterBoxTabWindow():m_bCheckAllState(FALSE) {}
	virtual ~CUILetterBoxTabWindow() {}

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh();
	LETTERLIST_TEXT * GetCurrentSelectedLetter();
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
	//CUIButton m_DeliveryButton;
	BOOL m_bCheckAllState;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIFriendWindow : public CUIBaseWindow
{
public:
	CUIFriendWindow():m_iTabIndex(0), m_iTabMouseOverIndex(0) {}
	virtual ~CUIFriendWindow() {}

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh();
	void Reset();
	void Close();
	void RefreshPalList() { m_FriendListWnd.RefreshPalList(); }
//	const char* GetCurrentSelectedFriend()	
//	{ return m_FriendListWnd.GetCurrentSelectedFriend(); }

	void AddWindow(DWORD dwUIID, const char* pszTitle)
	{ m_ChatRoomListWnd.AddWindow(dwUIID, pszTitle); }
	void RemoveWindow(DWORD dwUIID)
	{ m_ChatRoomListWnd.RemoveWindow(dwUIID); }
	DWORD GetCurrentSelectedWindow()
	{ return m_ChatRoomListWnd.GetCurrentSelectedWindow(); }
	void ResetWindow()
	{ m_ChatRoomListWnd.Reset(); }

	void RefreshLetterList() { m_LetterBoxWnd.RefreshLetterList(); }
	LETTERLIST_TEXT * GetCurrentSelectedLetter()
	{ return m_LetterBoxWnd.GetCurrentSelectedLetter(); }
	void PrevNextCursorMove(int iMove)
	{ m_LetterBoxWnd.PrevNextCursorMove(iMove); }

	void SetTabIndex(int iIndex) { m_iTabIndex = iIndex; }
	int GetTabIndex() { return m_iTabIndex; }

protected:
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
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUITextInputWindow : public CUIBaseWindow
{
public:
	CUITextInputWindow():m_dwReturnWindowUIID(0) {}
	virtual ~CUITextInputWindow() {}

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh();
	void SetText(const char* pszText) { m_TextInputBox.SetText(pszText); m_TextInputBox.GiveFocus(TRUE); }

protected:
	virtual void RenderSub();			// 하위 요소들을 그린다
	virtual BOOL HandleMessage();		// 메시지 처리 함수
	virtual void DoActionSub(BOOL bMessageOnly);			// 하위 요소들의 메시지, 마우스 액션 등을 처리한다.
	virtual void DoMouseActionSub();	// 하위 요소들의 마우스 액션을 처리한다.

	void ReturnText();	// 입력창의 값을 부모창에 전달하고 종료

protected:
	DWORD m_dwReturnWindowUIID;
	CUITextInputBox m_TextInputBox;
	CUIButton m_AddButton;
	CUIButton m_CancelButton;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIQuestionWindow : public CUIBaseWindow
{
public:
	CUIQuestionWindow(int iDialogType = 0):m_dwReturnWindowUIID(0), m_iDialogType(iDialogType) {}
	virtual ~CUIQuestionWindow() {}

	virtual void Init(const char* pszTitle, DWORD dwParentID = 0);
	virtual void Refresh();

	void SaveID(const char* pszText);

protected:
	virtual void RenderSub();
	virtual BOOL HandleMessage();
	virtual void DoActionSub(BOOL bMessageOnly);
	virtual void DoMouseActionSub();

protected:
	DWORD m_dwReturnWindowUIID;
	int m_iDialogType;	// 0: Y/N, 1: OK
	char m_szCaption[2][MAX_TEXT_LENGTH + 1];
	char m_szSaveID[MAX_ID_SIZE + 1];
	CUIButton m_AddButton;
	CUIButton m_CancelButton;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<DWORD, CUIBaseWindow *, std::less<DWORD> > WndMap;

class CUIWindowMgr : public CUIMessage
{
public:
	CUIWindowMgr();
	virtual ~CUIWindowMgr();

	void Reset();
	DWORD AddWindow(int iWindowType, int iPos_x, int iPos_y, const char* pszTitle, DWORD dwParentID = 0, int iOption = UIADDWND_NULL);
	void RemoveWindow(DWORD dwUIID);
	void Render();
	void DoAction();
	void ShowHideWindow(DWORD dwUIID, BOOL bShowWindow);
	void HideAllWindow(BOOL bHide, BOOL bMainClose = FALSE);
	void HideAllWindowClear();
	CUIBaseWindow * GetWindow(DWORD dwUIID);
	BOOL IsWindow(DWORD dwUIID);
	CUIFriendWindow * GetFriendMainWindow() { return (CUIFriendWindow *)GetWindow(m_dwMainWindowUIID); }
	void SetWindowsEnable(DWORD bWindowsEnable) { m_bWindowsEnable = bWindowsEnable; }
	BOOL GetWindowsEnable() { return m_bWindowsEnable; }
	DWORD GetTopWindowUIID() { return (m_WindowArrangeList.empty() == TRUE ? 0 : *m_WindowArrangeList.rbegin()); }
	DWORD GetTopNotMainWindowUIID();

	void AddWindowFinder(CUIBaseWindow * pWindow);
	void RemoveWindowFinder(DWORD dwUIID);
	void SendUIMessageToWindow(DWORD dwUIID, int iMessage, int iParam1, int iParam2);
	
	void OpenMainWnd(int iPos_x, int iPos_y);
	void CloseMainWnd();
	void RefreshMainWndPalList() { if (m_dwMainWindowUIID != 0) GetFriendMainWindow()->RefreshPalList(); }
	void RefreshMainWndLetterList()	{ if (m_dwMainWindowUIID != 0) GetFriendMainWindow()->RefreshLetterList(); }
	void RefreshMainWndChatRoomList();

	void SetChatReject(BOOL bChatReject) { m_bChatReject = bChatReject; }
	BOOL GetChatReject() { return m_bChatReject; }

	BOOL LetterReadCheck(DWORD dwLetterID);	// 편지 중복 읽기 방지
	void CloseLetterRead(DWORD dwLetterID);	// 편지 중복 읽기 방지
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
	void HandleMessage();			// 메시지 처리 함수

public:
	BOOL m_bRenderFrame;			// 프레임당 1회 액션을 위한 변수

protected:
	BOOL m_bWindowsEnable;
	DWORD m_dwMainWindowUIID;		// 창관리 윈도우의 UIID
	WndMap m_WindowMap;						// 번호순 윈도우 (메모리 생성/삭제 관리)
	WndMap m_WindowFindMap;					// 번호순 윈도우 (순수 검색용)
	WndMap m_WindowReadyMap;				// 대기중 윈도우 (대기했다가 m_WindowMap로 이동)
	WndMap::iterator m_WindowMapIter;
	std::list<DWORD> m_WindowArrangeList;		// 윈도우 정렬 순서 리스트 (front->back순서로 앞에 출력된다)
	std::list<DWORD>::iterator m_WindowArrangeListIter;
	std::list<DWORD>::reverse_iterator m_WindowReverseArrangeListIter;
	std::map<DWORD, DWORD, std::less<DWORD> > m_LetterReadMap;	// 편지 읽기 목록
	std::map<DWORD, DWORD, std::less<DWORD> >::iterator m_LetterReadMapIter;
	BOOL m_bCurrentHideWindowState;
	std::list<DWORD> m_HideWindowList;		// 숨겨진 윈도우 (복원용)
	std::list<DWORD> m_ForceTopWindowList;	// 최상위 윈도우 (서버에서)

	int m_iMainWindowPos_x, m_iMainWindowPos_y;
	int m_iMainWindowWidth, m_iMainWindowHeight;
	int m_iMainWindowBackPos_y, m_iMainWindowBackHeight;
	BOOL m_bIsMainWindowMaximize;
	BOOL m_bChatReject;
	int m_iLastFriendWindowTabIndex;

	BOOL m_bServerEnable;	// 채팅 서버 잘 되는가
	int m_iFriendMainWindowTitleNumber;	// 친구 메인창 이름 번호 (text.txt)
	DWORD m_dwAddWindowUIID;	// 친구 추가 윈도우 id
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIFriendMenu : public CUIBaseWindow
{
public:
	CUIFriendMenu() { Init(); }
	virtual ~CUIFriendMenu() { Reset(); }

	void Reset();
	void Init();
//	BOOL DoMouseAction();
//	void Render();
	void AddWindow(DWORD dwUIID, CUIBaseWindow * pWindow);
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

	DWORD CheckChatRoomDuplication(const char* pszTargetName);
	void SendChatRoomConnectCheck();
	void UpdateAllChatWindowInviteList();

	BOOL IsHotkeyEnable() { return m_bHotKey; }

	void AddRequestWindow(const char* szTargetName);
	BOOL IsRequestWindow(const char* szTargetName);
	void RemoveRequestWindow(const char* szTargetName);
	void RemoveAllRequestWindow();

	void CloseAllChatWindow();	// 대화 거부시 모든 채팅창 닫기
	void LockAllChatWindow();	// 모든 채팅창 잠그기
protected:
	virtual void RenderSub();			// 하위 요소들을 그린다
	virtual BOOL HandleMessage();		// 메시지 처리 함수
	virtual void DoActionSub(BOOL bMessageOnly);			// 하위 요소들의 메시지, 마우스 액션 등을 처리한다.
	virtual void DoMouseActionSub();	// 하위 요소들의 마우스 액션을 처리한다.

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
	std::deque<char*> m_RequestChatWindowList;
	std::deque<char*>::iterator m_RequestChatWindowListIter;
};

#endif	// __UIWINDOW_H__