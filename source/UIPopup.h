//  
//  UIPopup.h
//  
//  내  용 : 팝업창 띄우기
//  
//  날  짜 : 2004년 11월 12일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIPOPUP_H__1FD0B6C0_1EB3_4805_965C_C53A6A9A39B3__INCLUDED_)
#define AFX_UIPOPUP_H__1FD0B6C0_1EB3_4805_965C_C53A6A9A39B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// 팝업창의 종류
#define	POPUP_CUSTOM	0	// SetPopupExtraFunc 에 의해 처리되는 팝업창
#define	POPUP_OK		1	// 확인 버튼으로 이루어짐
#define	POPUP_OKCANCEL	2	// 확인, 취소 버튼으로 이루어짐
#define	POPUP_YESNO		4	// 예, 아니오 버튼으로 이루어짐
#define	POPUP_TIMEOUT	8	// 시간지나면 자동 닫기(조합가능)
#define	POPUP_INPUT		16	// 입력값을 받는 팝업

// 팝업창의 결과값
typedef int POPUP_RESULT;
#define	POPUP_RESULT_NONE		1
#define	POPUP_RESULT_OK			2			// 확인 버튼 눌림
#define	POPUP_RESULT_CANCEL		4			// 취소 버튼 눌림
#define	POPUP_RESULT_YES		8			// 예 버튼 눌림
#define	POPUP_RESULT_NO			16			// 아니오 버튼 눌림
#define	POPUP_RESULT_TIMEOUT	32			// 시간초과
#define	POPUP_RESULT_ESC		64			// ESC 눌림에 의한..(첨가값)

enum POPUP_ALIGN
{
	PA_CENTER,
	PA_TOP
};

#define MAX_POPUP_TEXTLINE		10
#define MAX_POPUP_TEXTLENGTH	256


class CUIPopup  
{
public:
	CUIPopup();
	virtual ~CUIPopup();

protected:
	// 팝업창의 결과값이 리턴될 함수포인터
	int (*PopupResultFuncPointer)( POPUP_RESULT Result );
	// 팝업창의 Custom 부분을 처리할 함수포인터
	void (*PopupUpdateInputFuncPointer)();
	void (*PopupRenderFuncPointer)();

	DWORD			m_dwPopupID;			// 고유 인덱스
	int				m_nPopupTextCount;		// 팝업텍스트 라인 수
	char			m_szPopupText[MAX_POPUP_TEXTLINE][MAX_POPUP_TEXTLENGTH];	// 팝업텍스트 내용
	int				m_PopupType;			// 팝업창 타입
	SIZE			m_sizePopup;			// 팝업창 크기
	POPUP_ALIGN		m_Align;				// 팝업창 뜨는 위치

	char			m_szInputText[1024];	// 입력 텍스트 버퍼 (POPUP_INPUT에서 쓰임)
	int				m_nInputSize;			// 입력창 크기 (POPUP_INPUT에서 쓰임)
	int				m_nInputTextLength;		// 입력텍스트 최대길이 (POPUP_INPUT에서 쓰임)
	UIOPTIONS		m_InputOptions;			// 입력스타일 (POPUP_INPUT에서 쓰임)

	DWORD			m_dwPopupStartTime;		// 팝업창이 시작된 시간 (POPUP_TIMEOUT에서 쓰임)
	DWORD			m_dwPopupEndTime;		// 팝업창이 종료될 시간 (POPUP_TIMEOUT에서 쓰임)
	DWORD			m_dwPopupElapseTime;	// 팝업창이 지속될 시간 (POPUP_TIMEOUT에서 쓰임)

	CUIButton		m_OkButton;
	CUIButton		m_CancelButton;
	CUIButton		m_YesButton;
	CUIButton		m_NoButton;

protected:
	bool CheckTimeOut();

public:
	// 초기화
	void Init();

	// 팝업설정
	DWORD SetPopup( const char* pszText, int nLineCount, int nBufferSize, int Type, int (*ResultFunc)( POPUP_RESULT Result ), POPUP_ALIGN Align=PA_CENTER );
	// Custom 팝업 (입력과 렌더링을 직접 처리할 경우 설정)
	void SetPopupExtraFunc( void (*InputFunc)(), void (*RenderFunc)() );

	void SetInputMode( int nSize, int nTextLength, UIOPTIONS Options );
	bool IsInputEnable();

	void SetTimeOut( DWORD dwElapseTime );

	char* GetInputText();
	DWORD GetPopupID();
	void Close();

	// 팝업창 강제 닫기
	void CancelPopup();

	// 내부에서 처리하면 TRUE 리턴
	bool PressKey( int nKey );

	// 입력처리
	void UpdateInput();
	// 렌더링
	void Render();
};

#endif // !defined(AFX_UIPOPUP_H__1FD0B6C0_1EB3_4805_965C_C53A6A9A39B3__INCLUDED_)
