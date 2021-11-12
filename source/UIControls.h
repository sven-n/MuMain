#ifndef __UICONTROL_H__
#define __UICONTROL_H__

#include "zzzinfomation.h"

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#include "MultiLanguage.h"
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#define UIMAX_TEXT_LINE			150
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

inline DWORD _ARGB(BYTE a, BYTE r, BYTE g, BYTE b) { return (a << 24) + (b << 16) + (g << 8) + (r); }

#ifndef ASG_ADD_NEW_QUEST_SYSTEM		// 정리시 삭제.
void CutText2(const char* Text,char* Text1,char* Text2,int Length);
#endif	// ASG_ADD_NEW_QUEST_SYSTEM
#ifdef KJH_FIX_BTS158_TEXT_CUT_ROUTINE
int CutStr(const char* pszText, char * pTextOut, const int iTargetPixelWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab = 0);
#endif // KJH_FIX_BTS158_TEXT_CUT_ROUTINE
int CutText3(const char* pszText, char* pTextOut, const int TargetWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab = 0, const BOOL bReverseWrite = FALSE);
// pszText : 원본 텍스트
// pTextOut : 반환될 텍스트 배열의 첫번째 주소 Array[4][100] -> Array[0]
// TargetWidth : 목표 너비
// iiMaxOutLine : 최대 라인 수
// iOutStrLength : 최대 텍스트 길이
// iFirstLineTab : 첫 라인 띄어 쓰기 (px)
// bReverseWrite : 나눈 텍스트를 배열 뒤에서부터 앞으로 저장
void CutText4(const char* pszSource, char* pszResult1, char* pszResult2, int iCutCount);

void RenderCheckBox(int iPos_x, int iPos_y, BOOL bFlag);

const int COORDINATE_TYPE_LEFT_TOP = 1;
const int COORDINATE_TYPE_LEFT_DOWN = 2;

BOOL CheckMouseIn(int iPos_x, int iPos_y, int iWidth, int iHeight, int CoordType = COORDINATE_TYPE_LEFT_TOP);

#define ID_UICEDIT 0x0001
#define SLIDEHELP_TIMER 1003

enum UISTATES
{
	UISTATE_NORMAL = 0, UISTATE_RESIZE, UISTATE_SCROLL, UISTATE_HIDE,
	UISTATE_MOVE, UISTATE_READY, UISTATE_DISABLE
};

enum UIOPTIONS
{
	UIOPTION_NULL = 0,				// 옵션 없음
	UIOPTION_NUMBERONLY = 1,		// 숫자만 입력 가능
	UIOPTION_SERIALNUMBER = 2,		// 숫자와 영대문자만 입력 가능
	UIOPTION_ENTERIMECHKOFF = 4,	// 엔터를 눌러도 IME 상태를 바꾸지 않음 (엔터 눌러도 사라지지 않는 입력창, 멀티라인등)
#ifdef LJH_ADD_RESTRICTION_ON_ID
	UIOPTION_PAINTBACK = 8,			// 배경을 검은색으로 칠해줌
	UIOPTION_NOLOCALIZEDCHARACTERS = 16	// 다국어지원용 아이디와 길드명 입력(영어로만 입력할 수 있게 기획)
#else  //LJH_ADD_RESTRICTION_ON_ID
	UIOPTION_PAINTBACK = 8			// 배경을 검은색으로 칠해줌
#endif //LJH_ADD_RESTRICTION_ON_ID

};

#ifndef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
const int MAX_LETTERTEXT_LENGTH = 1000;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
typedef struct
{
	BOOL	m_bIsSelected;
	char	m_szID[MAX_ID_SIZE+1];
	char	m_szText[MAX_TEXT_LENGTH + 1];
	int		m_iType;
	int		m_iColor;
	UINT	m_uiEmptyLines;
} WHISPER_TEXT;

typedef struct
{
	BOOL	m_bIsSelected;
	char	m_szID[MAX_ID_SIZE+1];
	BYTE	m_Number;
	BYTE	m_Server;
	BYTE	m_GuildStatus;
} GUILDLIST_TEXT;

typedef struct
{
	BOOL	m_bIsSelected;
	DWORD	m_dwUIID;		// 윈도우 ID
	char	m_szTitle[64];	// 윈도우 타이틀
	int		m_iStatus;		// 윈도우 상태
} WINDOWLIST_TEXT;

typedef struct
{
	BOOL	m_bIsSelected;
	DWORD	m_dwLetterID;	// 편지 번호 (임의)
	char	m_szID[MAX_ID_SIZE+1];	// 보낸사람
	char	m_szText[MAX_TEXT_LENGTH + 1];			// 제목
	char	m_szDate[16];			// 날짜
	char	m_szTime[16];			// 시간
	BOOL	m_bIsRead;				// 열람 여부
} LETTERLIST_TEXT;

typedef struct
{
	BOOL	m_bIsSelected;
	char	m_szText[MAX_LETTERTEXT_LENGTH + 1];
} LETTER_TEXT;

typedef struct
{
	BOOL	m_bIsSelected;
	char	m_szContent[60];
} GUILDLOG_TEXT;
typedef struct
{
	BOOL	m_bIsSelected;
	BYTE	GuildMark[64];
	char	szName[8+1];
	int		nMemberCount;
} UNIONGUILD_TEXT;

typedef struct
{
	BOOL	m_bIsSelected;
	char	szName[8+1];
	int		nCount;
	BYTE	byIsGiveUp;
	BYTE	bySeqNum;
} BCDECLAREGUILD_TEXT;

typedef struct tagMOVECOMMAND_TEXT
{
	BOOL	m_bIsSelected;
	BOOL	m_bCanMove;
	char	szMainMapName[32];		//. Main map name
	char	szSubMapName[32];		//. Substitute map name
	int		iReqLevel;				//. required level
	int		iReqZen;				//. required zen
	int		iGateNum;				//. Gate number
} MOVECOMMAND_TEXT;

typedef struct
{
	BOOL	m_bIsSelected;
	char	szName[8+1];
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

#ifdef ADD_SOCKET_MIX
typedef struct _SOCKETLIST_TEXT
{
	BOOL	m_bIsSelected;
	int		m_iSocketIndex;		// 소켓 번호
	char	m_szText[64 + 1];	// 소켓 정보 문자열
} SOCKETLIST_TEXT;
#endif	// ADD_SOCKET_MIX

////////////////////////////////////////////////////////////////////////////////////////////////////

enum UI_MESSAGE_ENUM
{							// Func						|Param1				|Param2
	UI_MESSAGE_NULL = 0,	// NULL

	// 윈도우 관리자 메시지
	UI_MESSAGE_SELECT,		// 윈도우 선택				|선택된 윈도우ID
	UI_MESSAGE_HIDE,		// 윈도우 숨김				|숨길 윈도우 ID
	UI_MESSAGE_MAXIMIZE,	// 윈도우 최대화			|최대화 시킬 윈도우 ID
	UI_MESSAGE_CLOSE,		// 윈도우 닫기				|닫을 윈도우ID
	UI_MESSAGE_BOTTOM,		// 윈도우 순서 맨 밑으로	|밑으로 보낼 윈도우 ID

	// 윈도우 메시지
	UI_MESSAGE_SELECTED,	// 윈도우 선택됨
	UI_MESSAGE_TEXTINPUT,	// 텍스트 입력  
	UI_MESSAGE_P_MOVE,		// (부모)윈도우가 이동
	UI_MESSAGE_P_RESIZE,	// (부모)윈도우가 크기조절
	UI_MESSAGE_BTNLCLICK,	// 버튼 클릭				|클릭된 버튼 ID
	UI_MESSAGE_TXTRETURN,	// 텍스트값 전달			|보내는 윈도우 ID	|전달할 텍스트 주소
							//												 (delete해줘야함)
	UI_MESSAGE_YNRETURN,	// 선택 값 전달				|보내는 윈도우 ID	|선택 값 (1:Yes, 2:No)

	// 리스트 메시지
	UI_MESSAGE_LISTDBLCLICK,// 리스트항목 더블클릭		|보내는 윈도우 ID
	UI_MESSAGE_LISTSCRLTOP,	// 리스트 스크롤 맨위로
	UI_MESSAGE_LISTSELUP,	// 리스트 선택 위로
	UI_MESSAGE_LISTSELDOWN	// 리스트 선택 아래로
};

struct UI_MESSAGE
{
	int m_iMessage;
	int m_iParam1;
	int m_iParam2;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIMessage
{
public:
	CUIMessage() {}
	virtual ~CUIMessage() { m_MessageList.clear(); }

	void SendUIMessage(int iMessage, int iParam1, int iParam2);
	void GetUIMessage();
protected:
	std::deque<UI_MESSAGE> m_MessageList;
	UI_MESSAGE m_WorkMessage;			// 현재 처리할 메시지 저장용
};
////////////////////////////////////////////////////////////////////////////////////////////////////

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
	virtual BOOL DoAction(BOOL bMessageOnly = FALSE);	// 메시지, 마우스 액션 등을 처리한다.

protected:
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DoActionSub()					{ return;	}
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DoActionSub(BOOL bMessageOnly) {			}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual BOOL DoMouseAction() { return TRUE; }
	virtual void DefaultHandleMessage();			// 기본 메시지 처리 함수
	virtual BOOL HandleMessage() { return FALSE; }	// 클래스별 메시지 처리 함수 (FALSE면 DefaultHandleMessage 실행)

protected:
	DWORD m_dwUIID;		// 고유 ID
	DWORD m_dwParentUIID;	// 부모 ID
	int m_iState;
	int m_iOptions;
	int m_iPos_x, m_iPos_y;
	int m_iWidth, m_iHeight;
	int m_iArrangeType;		// 정렬 타입 (0: 좌->우;상->하, 1: 우->좌;상->하, 2:좌->우;하->상, 3:우->좌;하->상)
	int m_iResizeType;		// 크기 조절 타입 (0: 절대;절대, 1: 상대;절대, 2: 절대;상대, 3: 상대;상대)
	int m_iRelativePos_x, m_iRelativePos_y;
	int m_iRelativeWidth, m_iRelativeHeight;
	int m_iCoordType;		// 좌표 타입
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIButton : public CUIControl
{
public:
	CUIButton();
	virtual ~CUIButton();

	virtual void Init(DWORD dwButtonID, const char* pszCaption);
	virtual void SetCaption(const char* pszCaption);
	virtual BOOL DoMouseAction();
	virtual void Render();

protected:
	DWORD m_dwButtonID;
	char* m_pszCaption;
	BOOL m_bMouseState;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

enum UILISTBOX_SCROLL_TYPE { UILISTBOX_SCROLL_DOWNUP = 0, UILISTBOX_SCROLL_UPDOWN };

template <class T>
class CUITextListBox : public CUIControl
{
public:
	CUITextListBox();
	virtual ~CUITextListBox();
	
	virtual void Clear();
	virtual void AddText() {}									// 데이터에 맞게 상속받아 사용

	virtual void Render();
	virtual void Scrolling(int iValue);

	virtual int GetBoxSize() { return m_iNumRenderLine; }
	virtual void SetBoxSize(int iLineNum) { m_iNumRenderLine = iLineNum; }
	virtual void SetNumRenderLine(int iLine) { m_iNumRenderLine = iLine; }
	virtual void Resize(int iValue);
	virtual BOOL HandleMessage();

	virtual void ResetCheckedLine(BOOL bFlag = FALSE);			// 라인 체크 초기화
	virtual BOOL HaveCheckedLine();								// 체크된 라인이 있나
	virtual int GetCheckedLines(std::deque<T*> * pSelectLineList);	// 체크된 라인들 얻기
	virtual int GetLineNum() { return (m_bUseMultiline == TRUE ? m_RenderTextList.size() : m_TextList.size()); }

	virtual void SLSetSelectLine(int iLineNum);					// 선택 라인 (1~라인수) , 0: 선택 없음
	virtual void SLSelectPrevLine(int iLineNum = 1);			// 이전 라인으로 이동 (-)
	virtual void SLSelectNextLine(int iLineNum = 1);			// 다음 라인으로 이동 (+)
	virtual typename std::deque<T>::iterator SLGetSelectLine();		// 현재 라인 데이터 얻기

	virtual int SLGetSelectLineNum() { return m_iSelectLineNum; }

protected:
	virtual BOOL DoMouseAction();
	virtual void RemoveText();									// 넘치는 텍스트 제거
	virtual void ComputeScrollBar();
	virtual void MoveRenderLine();								// 출력 시작 할 줄까지 m_TextListIter를 옮겨준다
	virtual BOOL CheckMouseInBox();								// 마우스 처리를 할 영역인가 체크

	virtual void RenderInterface() = 0;							// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber) = 0;			// 내부 텍스트 라인 출력
	virtual void RenderCoveredInterface() { }					// 최상단 인터페이스 출력
	virtual BOOL DoLineMouseAction(int iLineNumber) = 0;		// 내부 라인 마우스 액션
	virtual BOOL DoSubMouseAction() { return TRUE; }

	virtual void CalcLineNum() {}								// 줄 수 재계산 (멀티라인에서 사용)

protected:
	std::deque<T> m_TextList;
	typename std::deque<T>::iterator m_TextListIter;

	BOOL m_bUseSelectLine;				// 라인 선택 가능여부
	BOOL m_bPressCursorKey;				// 키 입력 처리용a
	int m_iSelectLineNum;				// 선택된 라인 번호

	BOOL m_bUseMultiline;		// 멀티라인 사용
	std::deque<T> m_RenderTextList;	// 출력용 (m_TextList 에서 가져온다)

	int m_iMaxLineCount;	// 최대 데이터수
	int m_iCurrentRenderEndLine;
	int m_iNumRenderLine;

	float m_fScrollBarRange_top;
	float m_fScrollBarRange_bottom;

	float m_fScrollBarPos_y;	// 0.0 ~ 1.0
	float m_fScrollBarWidth;
	float m_fScrollBarHeight;

	float m_fScrollBarClickPos_y;
	BOOL m_bScrollBtnClick;
	BOOL m_bScrollBarClick;

	int m_iScrollType;	// 스크롤 방향
	BOOL m_bNewTypeScrollBar;	// 스크롤바 새모양

	BOOL m_bUseNewUIScrollBar;	// 새로운 모양 스크롤바 사용 (버튼없고 스크롤바 드래그만 가능한)
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class CUIGuildListBox : public CUITextListBox<GUILDLIST_TEXT>	// 길드 리스트 창
{
public:
	CUIGuildListBox();
	virtual ~CUIGuildListBox() {}

	virtual void AddText(const char* pszID, BYTE Number, BYTE Server);
protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual BOOL DoLineMouseAction(int iLineNumber) { return TRUE; }	// 내부 라인 마우스 액션
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual BOOL DoSubMouseAction();
protected:
	BOOL m_bIsGuildMaster;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUISimpleChatListBox : public CUITextListBox<WHISPER_TEXT>	// 채팅 내용 리스트창 (윈도우,자동멀티라인)
{
public:
	CUISimpleChatListBox();
	virtual ~CUISimpleChatListBox() {}

	virtual void Render();
	virtual void AddText(const char* pszID, const char* pszText, int iType, int iColor);
protected:
	virtual void AddTextToRenderList(const char* pszID, const char* pszText, int iType, int iColor);
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual BOOL DoLineMouseAction(int iLineNumber)		{ return TRUE; }	// 내부 라인 마우스 액션
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			void CalcLineNum();	// 줄 수 계산
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUILetterTextListBox : public CUITextListBox<LETTER_TEXT>	// 편지 내용 리스트창 (윈도우,자동멀티라인)
{
public:
	CUILetterTextListBox();
	virtual ~CUILetterTextListBox() {}

	virtual void Render();
	virtual void AddText(const char* pszText);
protected:
	virtual void AddTextToRenderList(const char* pszText);
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual BOOL DoLineMouseAction(int iLineNumber) { return TRUE;	}	// 내부 라인 마우스 액션
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			void CalcLineNum();	// 줄 수 계산
	virtual int	GetRenderLinePos_y(int iLineNumber);
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIChatPalListBox : public CUITextListBox<GUILDLIST_TEXT>	// 채팅 상대 리스트창 (윈도우)
{
public:
	CUIChatPalListBox();
	virtual ~CUIChatPalListBox() {}

	virtual void AddText(const char* pszID, BYTE Number, BYTE Server);
	virtual void DeleteText(const char* pszID);
	virtual void SetNumRenderLine(int iLine);
	GUILDLIST_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
	std::deque<GUILDLIST_TEXT> & GetFriendList() { m_bForceEditList = TRUE; return m_TextList; }
	void SetLayout(int iType) { m_iLayoutType = iType; }
	const char* GetNameByNumber(BYTE byNumber);
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
	void MakeTitleText(char* pszTitleText);	// 채팅 방에 있는 사람들 이름 요약

protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);

protected:
	int m_iLayoutType;
	int m_iColumnWidth[4];	// 각 항목별 너비
	BOOL m_bForceEditList;	// 리스트 외부 수정
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUIWindowListBox : public CUITextListBox<WINDOWLIST_TEXT>	// 윈도우 리스트 창
{
public:
	CUIWindowListBox();
	virtual ~CUIWindowListBox() {}

	virtual void AddText(DWORD dwUIID, const char* pszTitle, int iStatus = 0);
	virtual void DeleteText(DWORD dwUIID);
	virtual void SetNumRenderLine(int iLine);
	WINDOWLIST_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);
};
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUILetterListBox : public CUITextListBox<LETTERLIST_TEXT>	// 편지 리스트 창
{
public:
	CUILetterListBox();
	virtual ~CUILetterListBox() {}

	virtual void AddText(const char* pszID, const char* pszText, const char* pszDate, const char* pszTime, BOOL bIsRead);
	virtual void DeleteText(DWORD dwLetterID);
	virtual void SetNumRenderLine(int iLine);
	LETTERLIST_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
	std::deque<LETTERLIST_TEXT> & GetLetterList() { m_bForceEditList = TRUE; return m_TextList; }
	
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
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);

protected:
	int m_iColumnWidth[4];	// 각 항목별 너비
	BOOL m_bForceEditList;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ADD_SOCKET_MIX
class CUISocketListBox : public CUITextListBox<SOCKETLIST_TEXT>	// 윈도우 리스트 창
{
public:
	CUISocketListBox();
	virtual ~CUISocketListBox() {}

	virtual void AddText(int iSocketIndex, const char * pszText);
	virtual void DeleteText(int iSocketIndex);
	virtual void SetNumRenderLine(int iLine);
	SOCKETLIST_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif	// ADD_SOCKET_MIX

// 길드공지 리스트창
class CUIGuildNoticeListBox : public CUITextListBox<GUILDLOG_TEXT>
{
public:
	CUIGuildNoticeListBox();
	virtual ~CUIGuildNoticeListBox() {}

	virtual void AddText( const char* szContent );
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DeleteText( DWORD dwIndex );
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void SetNumRenderLine( int nLine );
	GUILDLOG_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);
};

// 길드멤버 리스트창
class CUINewGuildMemberListBox : public CUITextListBox<GUILDLIST_TEXT>
{
public:
	CUINewGuildMemberListBox();
	virtual ~CUINewGuildMemberListBox() {}

	virtual void AddText(const char* pszID, BYTE Number, BYTE Server, BYTE GuildStatus);
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DeleteText(DWORD dwUIID);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void SetNumRenderLine(int iLine);
	GUILDLIST_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);
protected:
	BOOL m_bIsGuildMaster;
};

// 연합길드 리스트
class CUIUnionGuildListBox : public CUITextListBox<UNIONGUILD_TEXT>	// 용병길드 리스트 창

{
public:
	CUIUnionGuildListBox();
	virtual ~CUIUnionGuildListBox() {}

	virtual void AddText(BYTE* pGuildMark, const char* szGuildName, int nMemberCount);
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DeleteText(DWORD dwGuildIndex);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual int GetTextCount();
	virtual void SetNumRenderLine(int iLine);
	UNIONGUILD_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);
};

// 보석 조합 리스트 창
class CUIUnmixgemList : public CUITextListBox<UNMIX_TEXT>
{
public:
	CUIUnmixgemList();
	virtual ~CUIUnmixgemList() {}
	virtual void SetNumRenderLine(int iLine);
	UNMIX_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
	virtual void AddText( int iIndex, BYTE cComType );
	void Sort();

	inline bool IsNotified() { return m_bNotify; }
	inline bool IsEmpty() { return m_TextList.empty(); }

protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);

	bool	m_bNotify;
};

// 공성전 선포한 길드리스트
class CUIBCDeclareGuildListBox : public CUITextListBox<BCDECLAREGUILD_TEXT>

{
public:
	CUIBCDeclareGuildListBox();
	virtual ~CUIBCDeclareGuildListBox() {}

	virtual void AddText( const char* szGuildName, int nMarkCount, BYTE byIsGiveUp, BYTE bySeqNum );
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DeleteText(DWORD dwGuildIndex);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void SetNumRenderLine(int iLine);
	BCDECLAREGUILD_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
	void Sort();

protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);
};
// 공성전 확정된 길드리스트
class CUIBCGuildListBox : public CUITextListBox<BCGUILD_TEXT>

{
public:
	int		Select_Guild;
	CUIBCGuildListBox();
	virtual ~CUIBCGuildListBox() {}

	virtual void AddText( const char* szGuildName, BYTE byJoinSide, BYTE byGuildInvolved 	
		,int iGuildScore
		);
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void DeleteText(DWORD dwGuildIndex);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual void SetNumRenderLine(int iLine);
	BCGUILD_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
protected:
	virtual void RenderInterface();	// 인터페이스 출력
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int iLineNumber);
};

//-------------------------------------------------------------------------------------------
// 이동명령창 리스트박스
class CUIMoveCommandListBox : public CUITextListBox<MOVECOMMAND_TEXT>
{
public:
	CUIMoveCommandListBox();
	virtual ~CUIMoveCommandListBox() {}

	virtual void AddText( int iIndex, const char* szMapName, const char* szSubMapName, int iReqLevel, int iReqZen, int iGateNum );
	//virtual void DeleteText(DWORD dwGuildIndex);
	virtual void SetNumRenderLine(int iLine);
	MOVECOMMAND_TEXT * GetSelectedText() { return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
	//void Sort();

protected:
	virtual void RenderInterface();	// 인터페이스 출력
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual BOOL RenderDataLine(int iLineNumber);	// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int iLineNumber);	// 내부 라인 마우스 액션
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual int GetRenderLinePos_y(int iLineNumber);
};
//-------------------------------------------------------------------------------------------

#ifdef ASG_MOD_UI_QUEST_INFO
// 진행 중인 퀘스트 항목 구조체.
struct SCurQuestItem
{
	BOOL	m_bIsSelected;
	DWORD	m_dwIndex;
	char	m_szText[64];
};

// 진행 중인 퀘스트 리스트 박스 UI 클래스
class CUICurQuestListBox : public CUITextListBox<SCurQuestItem>
{
public:
	CUICurQuestListBox();
	virtual ~CUICurQuestListBox() {}
	
	virtual void AddText(DWORD dwQuestIndex, const char* pszText);
	virtual void DeleteText(DWORD dwQuestIndex);
	virtual void SetNumRenderLine(int nLine);
	SCurQuestItem* GetSelectedText()
	{ return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
	
protected:
	virtual void RenderInterface();				// 인터페이스 출력
	virtual BOOL RenderDataLine(int nLine);		// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int nLine);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int nLine);
};

enum REQUEST_REWARD_CLASSIFY;

struct SQuestContents
{
	BOOL	m_bIsSelected;
	HFONT	m_hFont;
	DWORD	m_dwColor;
	int		m_nSort;
	char	m_szText[64];
	REQUEST_REWARD_CLASSIFY	m_eRequestReward;
	DWORD	m_dwType;
	WORD	m_wIndex;
	ITEM*	m_pItem;
};

struct SRequestRewardText;

class CUIQuestContentsListBox : public CUITextListBox<SQuestContents>
{
public:
	CUIQuestContentsListBox();
	virtual ~CUIQuestContentsListBox() {}

	virtual void AddText(HFONT hFont, DWORD dwColor, int nSort, const char* pszText);
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

#endif	// ASG_MOD_UI_QUEST_INFO

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
	char m_szText[MAX_TEXT_LENGTH + 1];
	int m_iWidth;
	int m_iHeight;
	int m_iTab;				// 탭 크기
	DWORD m_dwUseCount;		// 삭제용 사용 카운트
	DWORD m_dwTextColor;	// 사용된 글자색과 배경색
	DWORD m_dwBackColor;
	int m_iTextureIndex;	// 텍스트 텍스쳐 번호
	int m_iTextureIndexEx;
	BOOL m_bUseTextEX;		// 확장 텍스트 그래픽 사용 여부 (256px 이상)
};
////////////////////////////////////////////////////////////////////////////////////////////////////
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

	virtual void RenderText(int iPos_x, int iPos_y, const char* pszText, int iBoxWidth = 0, int iBoxHeight = 0, 
		int iSort = RT3_SORT_LEFT, OUT SIZE* lpTextSize = NULL) = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::multimap<int, RENDER_TEXT_DATA, std::less<int> > RTMap;
////////////////////////////////////////////////////////////////////////////////////////////////////
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
#ifdef KJH_FIX_UI_CHAT_MESSAGE
	DWORD ConvertColorToDword(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha);
#endif // KJH_FIX_UI_CHAT_MESSAGE

	void SetFont(HFONT hFont);

	void RenderText(int iPos_x, int iPos_y, const unicode::t_char* pszText, int iBoxWidth = 0, int iBoxHeight = 0, 
		int iSort = RT3_SORT_LEFT, OUT SIZE* lpTextSize = NULL);

protected:
	void WriteText(int iOffset, int iWidth, int iHeight);
	void UploadText(int sx,int sy,int Width,int Height);
};
////////////////////////////////////////////////////////////////////////////////////////////////////
class CUIRenderText
{
	CUIRenderText();

	IUIRenderText*	m_pRenderText;
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

	void RenderText(int iPos_x, int iPos_y, const char* pszText, int iBoxWidth = 0, int iBoxHeight = 0, 
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
	virtual void SetText(const char* pszText);
	virtual void GetText(char* pszText, int iGetLenght = MAX_TEXT_LENGTH);

	virtual void GetText(wchar_t* pwszText, int iGetLenght = MAX_TEXT_LENGTH);
	HWND GetHandle() { return m_hEditWnd; }
	HWND GetParentHandle() { return m_hParentWnd; }
	BOOL HaveFocus() { return (GetHandle() == GetFocus()); }
	BOOL UseMultiline() { return m_bUseMultiLine; }
	virtual void SetTabTarget(CUITextInputBox * pTabTarget) {  m_pTabTarget = pTabTarget; }
	CUITextInputBox * GetTabTarget() { return m_pTabTarget; }

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
	void UploadText(int sx,int sy,int Width,int Height);

public:
	WNDPROC m_hOldProc;
	int m_iCaretBlinkTemp;

protected:
	HWND m_hParentWnd;
	HWND m_hEditWnd;
	HDC m_hMemDC;
	HBITMAP m_hBitmap;
	BYTE* m_pFontBuffer;

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
	void GetTexts(char* pText, char* pBuddyText);
	void ClearTexts();
	void SetText(BOOL bSetText, const char* pText, BOOL bSetBuddyText, const char* pBuddyText);
	void SetState(int iState);
	int GetState() { return m_TextInputBox.GetState(); }
	void SetFont(HFONT hFont);
	void SetTextPosition(int iPos_x, int iPos_y) { m_TextInputBox.SetPosition(iPos_x, iPos_y); }
	void SetBuddyPosition(int iPos_x, int iPos_y) { m_BuddyInputBox.SetPosition(iPos_x, iPos_y); }
	BOOL HaveFocus() { return (m_TextInputBox.HaveFocus() || m_BuddyInputBox.HaveFocus()); }
	BOOL DoMouseAction();
#if defined FOR_WORK || defined USER_WINDOW_MODE || (defined WINDOWMODE)
	void RestoreFocus() { m_bFocusLose = TRUE; }
#endif

protected:
	CUITextInputBox m_TextInputBox;
	CUITextInputBox m_BuddyInputBox;
#if defined FOR_WORK || defined USER_WINDOW_MODE || (defined WINDOWMODE)
	BOOL m_bFocusLose;
	int m_iBackupFocus;
#endif

public:
	virtual void AddHistory(const char* pszText);
	virtual void MoveHistory(int iDegree);
private:
	void RemoveHistory(BOOL bClear);
	BOOL m_bHistoryMode;
	char m_szTempText[MAX_TEXT_LENGTH + 1];
	std::deque<char*> m_HistoryList;
	std::deque<char*>::iterator m_CurrentHistoryLine;
	std::deque<char*>::iterator m_HistoryListIter;
};

class CUILoginInputBox : public CUIChatInputBox
{
public:
	CUILoginInputBox() {}
	virtual ~CUILoginInputBox() {}
	virtual void Init(HWND hWnd);
	virtual void AddHistory(const char* pszText) {}
	virtual void MoveHistory(int iDegree) {}

};

class CUIMercenaryInputBox : public CUIChatInputBox
{
public:
	CUIMercenaryInputBox() {}
	virtual ~CUIMercenaryInputBox() {}
	virtual void Init(HWND hWnd);
	virtual void AddHistory(const char* pszText) {}
	virtual void MoveHistory(int iDegree) {}
};

#ifdef REVISION_SLIDE_LEVEL_MAX
#define SLIDE_LEVEL_MAX 10 
#else //REVISION_SLIDE_LEVEL_MAX
#define SLIDE_LEVEL_MAX 5
#endif //REVISION_SLIDE_LEVEL_MAX

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
	char* m_pszText;
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
	BOOL AddSlideText(const char* pszNewText, DWORD dwTextColor = (255<<24)+(200<<16)+(220<<8)+(230));

	int GetAlphaRate() { return m_iAlphaRate; }
	BOOL HaveText();

	void AddSlide(int iLoopCount, int iLoopDelay, const char* pszText, int iType, float fSpeed, DWORD dwTextColor);
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

	int CheckCutSize(const char* pszSource, int iNeedValue);				// 자르는 사이즈 계산
	
protected:
	HFONT m_hFont;
	char* m_pszSlideText;
	char m_szSlideTextA[SLIDE_TEXT_LENGTH];
	char m_szSlideTextB[SLIDE_TEXT_LENGTH];
	float m_fMovePosition;
	float m_fMoveAccel;
	float m_fMoveSpeed;
	float m_fMaxMoveSpeed;
	int m_iAlphaRate;
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
	void OpenSlideTextFile(const char* szFileName);
	void ClearSlideText();
	const char* GetSlideText(int iLevel);
	void SetCreateDelay(int iDelay) { m_iCreateDelay = iDelay; }

	void AddSlide(int iLoopCount, int iLoopDelay, const char* pszText,
		int iType, float fSpeed, DWORD dwTextColor = (255<<24)+(200<<16)+(220<<8)+(230));
	void ManageSlide();
	BOOL IsIdle();
protected:
	CUISlideHelp m_HelpSlide;
	CUISlideHelp m_NoticeSlide;

	int m_iCreateDelay;
	int m_iLevelCap[SLIDE_LEVEL_MAX];
	int m_iTextNumber[SLIDE_LEVEL_MAX];
	std::list<char*> m_SlideTextList[SLIDE_LEVEL_MAX];
	std::list<char*>::iterator m_SlideTextListIter;
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
	
	unicode::t_char	m_szName[MAX_TEXT_LENGTH];
	unicode::t_char m_szNum[MAX_TEXT_LENGTH];
	unicode::t_char m_szPeriod[MAX_TEXT_LENGTH];
	unicode::t_char	m_szSendUserName[MAX_ID_SIZE+1];
	unicode::t_char	m_szMessage[MAX_GIFT_MESSAGE_SIZE];
	unicode::t_char m_szType;
}IGS_StorageItem;

// 선물함 보관함의 리스트 박스 UI 클래스
class CUIInGameShopListBox : public CUITextListBox<IGS_StorageItem>
{
	enum IMAGE_LISTBOX_SIZE
	{
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		LISTBOX_WIDTH =146,
#else // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		LISTBOX_WIDTH =140,
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
		LISTBOX_HEIGHT =115,
	};
public:
	CUIInGameShopListBox();
	virtual ~CUIInGameShopListBox() {}
	
	virtual void AddText(IGS_StorageItem& _StorageItem);
	virtual void SetNumRenderLine(int nLine);
	IGS_StorageItem* GetSelectedText()
	{ return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT				// #ifndef
	void DelData(int iIndex);		// Data하나를 지운다.
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	
protected:
	virtual void RenderInterface();				// 인터페이스 출력
	virtual BOOL RenderDataLine(int nLine);		// 내부 텍스트 라인 출력
	virtual BOOL DoLineMouseAction(int nLine);	// 내부 라인 마우스 액션
	virtual int GetRenderLinePos_y(int nLine);
};


#define LINE_TEXTMAX	64
// 최대 10줄정도의 택스트로 제한하자.확장해도 상관없음.800x600에서의
#define INFO_LINEMAX	10
// FULLSCREEN의 최대 사이즈
#define INFO_LINE_CNTMAX	50


struct IGS_BuyList
{
	BOOL	m_bIsSelected;
	char	m_pszItemExplanation[LINE_TEXTMAX];	// 상품 설명
};

// 단일 구매창의 리스트 박스 UI클래스
class CUIBuyingListBox : public CUITextListBox<IGS_BuyList>
{
	enum IMAGE_LISTBOX_SIZE
	{
		LISTBOX_WIDTH = 175,		// 리스트 박스 사이즈
		LISTBOX_HEIGHT = 95,
	};
public:
	CUIBuyingListBox();
	virtual ~CUIBuyingListBox() {}

	virtual void AddText(const char* pszExplanationText);
	virtual void SetNumRenderLine(int nLine);
	IGS_BuyList* GetSelectedText()
	{ return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }

	// 텍스트 라인 선택시 음영을 안주기 위해 사용 default 는 색을 넣는다
	void SetLineColorRender(const bool _LineColor = true);
	const bool& GetLineColorRender() const { return m_bRenderLineColor; }

protected:
	virtual void RenderInterface();
	virtual BOOL RenderDataLine(int nLine);
	virtual BOOL DoLineMouseAction(int nLine);
	virtual int GetRenderLinePos_y(int nLine);

	// 텍스트 라인색 그리기
	bool m_bRenderLineColor;
};
// listbox내에서 사용하는 체크 버튼 3모션용
// iter 에서 객체를 갖고 사용하자
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
		LBTN_DEFAULT=0,
		LBTN_UP,
		LBTN_DOWN,
	};
public:
	CRadioButton();
	~CRadioButton();
	// 마우스 상태
	bool UpdateMouseAction();
	// 클릭상태에 따른 라디오버튼 상태
	bool UpdateActionCheck(int _nState);
	// 마우스 상태에 따라 랜더
	void RadiobuttonBoxRender();
	// 버튼의 rect
	void SetRadioBtnRect(float _x=0, float _y=0, float _width=BTN_WIDTH, float _height=BTN_HEIGHT);
	bool IsRadioBtn(RECT _rt);
	void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv);
	// 체크 된상태
	const bool& GetCheckBtn() const { return m_bCheckState; }

	///////////////////////////////////////////////////////////
	// 이부분의 기능을 제거하면 check버튼으로 사용가능
	// static 변수로 현재 클릭된 인덱스 번호를 기억한다
	static int m_nIterIndex;
	// 클래스 생성시에 생성된 인덱스를 갖는다
	int m_nRadioBtnEnable;
	void SetCheckState(bool _Value);
	void SetRadioBtnIsEnable(int _Value);
	int GetRadioBtnIsEnable(){ return m_nRadioBtnEnable; }
	///////////////////////////////////////////////////////////
private:
	RECT m_rtCheckBtn;
	// 체크 상태
	bool m_bCheckState;
	// 마우스 눌린상태
	BYTE m_byMouseState;	
};
// 패키지 구매창의 리스트 박스 UI클래스
struct IGS_PackBuyList
{
	BOOL	m_bIsSelected;
	char	m_pszItemName[32];	// 아이템명
	char	m_nItemInfo[32];	// 기간및 수량 문자열로 받는다
	CRadioButton m_CheckBtn;	// 체크 버튼 클래스객체
};

typedef struct 
{
	BOOL	m_bIsSelected;
	int		m_iPackageSeq;
	int		m_iDisplaySeq;
	int		m_iPriceSeq;
	WORD	m_wItemCode;
#ifndef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL			// #ifndef
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	int		m_iCashType;		// 캐시타입 (C/P) - 글로벌 전용
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL

	unicode::t_char m_szItemName[MAX_TEXT_LENGTH];
	unicode::t_char m_szItemPrice[MAX_TEXT_LENGTH];
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	unicode::t_char m_szItemPriceUnit[MAX_TEXT_LENGTH];
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	unicode::t_char m_szItemPeriod[MAX_TEXT_LENGTH];
	unicode::t_char m_szAttribute[MAX_TEXT_LENGTH];

	CRadioButton m_RadioBtn;	//라디오 버튼 사용
}IGS_SelectBuyItem;

class CUIPackCheckBuyingListBox : public CUITextListBox<IGS_SelectBuyItem>
{
	enum IMAGE_LISTBOX_SIZE
	{
		LISTBOX_WIDTH = 180,		// 리스트 박스 사이즈
		LISTBOX_HEIGHT = 100,
		TEXT_HEIGHTSIZE = 33,
	};
public:
	CUIPackCheckBuyingListBox();
	virtual ~CUIPackCheckBuyingListBox();

	virtual void AddText(IGS_SelectBuyItem& Item);
	virtual void SetNumRenderLine(int iLine);
	IGS_SelectBuyItem* GetSelectedText()
	{ return (SLGetSelectLine() == m_TextList.end() ? NULL : &(*SLGetSelectLine())); }
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
