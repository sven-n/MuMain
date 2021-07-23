// NewUIMoveCommandWindow.h: interface for the CNewUIMoveCommandWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMOVECOMMANDWINDOW_H__E6ABC928_0A79_41CD_8046_5ED052B1985A__INCLUDED_)
#define AFX_NEWUIMOVECOMMANDWINDOW_H__E6ABC928_0A79_41CD_8046_5ED052B1985A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "MoveCommandData.h"
#include "NewUIChatLogWindow.h"

#ifdef YDG_ADD_ENC_MOVE_COMMAND_WINDOW

namespace SEASON3B
{
	class CNewUIMoveCommandWindow : public CNewUIObj
	{
#ifdef YDG_MOD_PROTECT_AUTO_V4
		friend class CMoveCommandWindowEncrypt;
#endif	// YDG_MOD_PROTECT_AUTO_V4
		enum IMAGE_LIST
		{
			IMAGE_MOVECOMMAND_SCROLL_TOP	= CNewUIChatLogWindow::IMAGE_SCROLL_TOP,			// newui_scrollbar_up.tga (7,3)
			IMAGE_MOVECOMMAND_SCROLL_MIDDLE	= CNewUIChatLogWindow::IMAGE_SCROLL_MIDDLE,			// newui_scrollbar_m.tga (7,15)
			IMAGE_MOVECOMMAND_SCROLL_BOTTOM	= CNewUIChatLogWindow::IMAGE_SCROLL_BOTTOM,			// newui_scrollbar_down.tga (7,3)
			IMAGE_MOVECOMMAND_SCROLLBAR_ON	= CNewUIChatLogWindow::IMAGE_SCROLLBAR_ON,			// newui_scroll_On.tga (15,30)	
			IMAGE_MOVECOMMAND_SCROLLBAR_OFF	= CNewUIChatLogWindow::IMAGE_SCROLLBAR_OFF,			// newui_scroll_Off.tga (15,30)
			//IMAGE_MOVECOMMAND_DRAG_BTN		= CNewUIChatLogWindow::IMAGE_DRAG_BTN			
		};
		
		enum 
		{
			MOVECOMMAND_SCROLLBTN_WIDTH = 15,
			MOVECOMMAND_SCROLLBTN_HEIGHT = 30,
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH = 7,
			MOVECOMMAND_SCROLLBAR_TOP_HEIGHT = 3,
			MOVECOMMAND_SCROLLBAR_MIDDLE_WIDTH = 7,
			MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT = 15,

			// 밑에 값은 기본 초기값으로만 사용하고 실제 적용은 m_iTextLine 변수로 이루어 진다.
			MOVECOMMAND_MAX_RENDER_TEXTLINE = 16,		// 이동명령 정보창에 보여지는 텍스트갯수를 여기서 정한다.(Default: 16)
		};

		enum MOVECOMMAND_MOUSE_EVENT
		{
			MOVECOMMAND_MOUSEBTN_NORMAL = 0,
			MOVECOMMAND_MOUSEBTN_OVER,
			MOVECOMMAND_MOUSEBTN_CLICKED,
		};

	protected:
//$$AUTO_BUILD_LINE_ SHUFFLE_BEGIN	// 섞기기능 임시로 껐음
		CNewUIManager*				m_pNewUIMng;
		POINT						m_Pos;

		int							m_iRealFontHeight;
			
		std::list<CMoveCommandData::MOVEINFODATA*>	m_listMoveInfoData;
		POINT						m_StartUISubjectName;	// UI제목 위치
		POINT						m_StartMapNamePos;		// 맵 이름 시작 위치
		POINT						m_MapNameUISize;		// 맵 이동명령UI SIZE
#ifdef ASG_ADD_GENS_SYSTEM
		POINT						m_StrifePos;			// 분쟁지역
#endif	// ASG_ADD_GENS_SYSTEM
		POINT						m_MapNamePos;
		POINT						m_ReqLevelPos;
		POINT						m_ReqZenPos;
		int							m_iSelectedMapName;		// 선택된 맵인덱스

		POINT						m_ScrollBarPos;
		POINT						m_ScrollBtnStartPos;
		POINT						m_ScrollBtnPos;

		int							m_iScrollBarHeightPixel;	// 스크롤바 길이
		int							m_iRenderStartTextIndex;	// UI에 렌더되는 처음 Text의 인덱스
		int							m_iRenderEndTextIndex;		// UI에 렌더되는 마지막 Text의 인덱스
		int							m_iSelectedTextIndex;		// 현재 선택된 Text
		int							m_iScrollBtnInterval;		// 스크롤바 움직이는 간격
		int							m_iScrollBarMiddleNum;				// 스크롤바 Middle 갯수
		int							m_iScrollBarMiddleRemainderPixel;	// 스크롤바 Middle갯수대로 렌더후 남은 픽셀
		int							m_iNumPage;					// 전체페이지
		int							m_iCurPage;					// 현재페이지
		int							m_iTotalMoveScrBtnPixel;			// 스크롤버튼의 전체 픽셀값
		int							m_iRemainMoveScrBtnPixel;		// 스크롤버튼의 이동해야할 나머지 픽셀값
		int							m_icurMoveScrBtnPixelperStep;	// 스크롤버튼의 현재 이동 픽셀값
		int							m_iMaxMoveScrBtnPixelperStep;	// 스크롤버튼의 이동하는 최대픽셀값
		int							m_iMinMoveScrBtnPixelperStep;	// 스크롤버튼의 이동하는 최소픽셀값
		int							m_iTotalMoveScrBtnperStep;		// 스크롤버튼의 이동해야할 전체 칸의 갯수
		int							m_iRemainMoveScrBtnperStep;		// 스크롤버튼의 이동해야할 나머지 칸의 갯수
		int							m_iTotalNumMaxMoveScrBtnperStep;	// 스크롤버튼의 최대이동칸의 전체 갯수
		int							m_iTotalNumMinMoveScrBtnperStep;	// 스크롤버튼의 최소이동칸의 전체 갯수
		int							m_iAcumMoveMouseScrollPixel;		// 마우스 스크롤값 누적(한칸 이동할때 쓰인다. ScrollUp/ScrollDown)
		int							m_iMousePosY;
		int							m_iScrollBtnMouseEvent;				// 스크롤 버튼 마우스 클릭
		bool						m_bScrollBtnActive;					// 스크롤버튼 활성화

		int							m_iPosY_Random;			// Y위치 랜덤값
		int							m_iPosY_Random_Min;		// Y위치 랜덤 최소값 
		int							m_iPosY_Random_Range;	// Y위치 랜덤값 범위
		POINT						m_Pos_Start;			// 창 생성 시작 위치 Create함수 호출 할 때 입력 받은 값
		int							m_iWindowOpenCount;		// 창 연 횟수
		int							m_iWorldIndex;			// 맵 인덱스
		bool						m_bCastleOwner;			// 수성측 길드나 연합이면 true 아니면 false
		int							m_iTextLine;			// 화면에 표시될 텍스트 라인수
		int							m_iTextAlpha;			// 텍스트 알파값
		int							m_iTopSpace;			// 상단 여백
		int							m_iWheelCounter;		// 먹통 처리용 휠 카운터
//$$AUTO_BUILD_LINE_ SHUFFLE_END
		float						m_fBackgroundAlpha;		// 배경 알파값
#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
		BYTE *						m_pbtKeyAddressBlock3;
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
		DWORD						m_dwMoveCommandKey;		// 이동 명령 전송시 키 값
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

	public:
		CNewUIMoveCommandWindow();
		virtual ~CNewUIMoveCommandWindow();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		bool BtnProcess();
		
		float GetLayerDepth();	//. 6.4f
		
		virtual void OpenningProcess();
		void ClosingProcess();

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
		void ResetWindowOpenCount();
		void SetCastleOwner(bool bOwner);
		bool IsCastleOwner();
#endif // CSK_MOD_MOVE_COMMAND_WINDOW

#ifdef CSK_LUCKY_SEAL
		bool IsLuckySealBuff();
		bool IsMapMove( const string& src );
#endif //CSK_LUCKY_SEAL

#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
		void SetMoveCommandKey(DWORD dwKey);	// 초기값 설정
		DWORD GetMoveCommandKey();				// 초기값을 바탕으로 키값을 만들어낸다 [주의!] SendRequestMoveMap 한번 보낼때 한번만 호출되어야 한다!
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

#ifdef LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
		BOOL IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const;
#endif //LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		int GetMapIndexFromMovereq(const char *pszMapName);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	protected:
#ifdef ASG_ADD_GENS_SYSTEM
		void SetStrifeMap();
#endif	// ASG_ADD_GENS_SYSTEM
		void SettingCanMoveMap();
		void RenderFrame();
		//void MoveScrollBtn(int iMoveValue);
		void UpdateScrolling();
		void ScrollUp(int iMoveValue);
		void ScrollDown(int iMoveValue);
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
		// bSign : true(DownScroll,+), false(UpScroll,-)
		void RecursiveCalcScroll(IN int piScrollValue, OUT int* piMovePixel, bool bSign = true);
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
		void LoadImages();
		void UnloadImages();
	};
	
#ifdef YDG_MOD_PROTECT_AUTO_V4
	class CMoveCommandWindowEncrypt
	{
	public:
		CMoveCommandWindowEncrypt();
		virtual ~CMoveCommandWindowEncrypt();
		static void Enable(BOOL bFlag);	// CMoveCommandWindow가 초기화 될때 한번만 호출
#ifdef YDG_MOD_PROTECT_AUTO_V4_R2
#ifdef YDG_MOD_PROTECT_AUTO_V4_R4
		static int __fastcall Read(int * piValue);
		static LONG __fastcall Read(LONG * plValue);
		static void __fastcall Write(int * piValue, int iValue);
		static void __fastcall Write(LONG * plValue, LONG lValue);
#else	// YDG_MOD_PROTECT_AUTO_V4_R4
		static int Read(int * piValue);
		static LONG Read(LONG * plValue);
		static void Write(int * piValue, int iValue);
		static void Write(LONG * plValue, LONG lValue);
#endif	// YDG_MOD_PROTECT_AUTO_V4_R4
#endif	// YDG_MOD_PROTECT_AUTO_V4_R2
#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
		static void CreateKey(BOOL bInit = FALSE);
		static void DeleteKey();
		static void CreateFakeKey();
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3

	protected:
		static void Encrypt(int * piValue);
		static void Decrypt(int * piValue);
		static void Encrypt(POINT * pValue);
		static void Decrypt(POINT * pValue);
		static void EncryptWindowPos();
		static void DecryptWindowPos();

	protected:
		static DWORD m_dwEncryptSeed;
		static int m_iEncryptCounter;	// 0: 암호화, 1+:암호 해제상태
	};
#endif	// YDG_MOD_PROTECT_AUTO_V4
};

#else	// YDG_ADD_ENC_MOVE_COMMAND_WINDOW































































namespace SEASON3B
{
	class CNewUIMoveCommandWindow : public CNewUIObj
	{
#ifdef YDG_MOD_PROTECT_AUTO_V4
		friend class CMoveCommandWindowEncrypt;
#endif	// YDG_MOD_PROTECT_AUTO_V4
		enum IMAGE_LIST
		{
			IMAGE_MOVECOMMAND_SCROLL_TOP	= CNewUIChatLogWindow::IMAGE_SCROLL_TOP,			// newui_scrollbar_up.tga (7,3)
			IMAGE_MOVECOMMAND_SCROLL_MIDDLE	= CNewUIChatLogWindow::IMAGE_SCROLL_MIDDLE,			// newui_scrollbar_m.tga (7,15)
			IMAGE_MOVECOMMAND_SCROLL_BOTTOM	= CNewUIChatLogWindow::IMAGE_SCROLL_BOTTOM,			// newui_scrollbar_down.tga (7,3)
			IMAGE_MOVECOMMAND_SCROLLBAR_ON	= CNewUIChatLogWindow::IMAGE_SCROLLBAR_ON,			// newui_scroll_On.tga (15,30)	
			IMAGE_MOVECOMMAND_SCROLLBAR_OFF	= CNewUIChatLogWindow::IMAGE_SCROLLBAR_OFF,			// newui_scroll_Off.tga (15,30)
			//IMAGE_MOVECOMMAND_DRAG_BTN		= CNewUIChatLogWindow::IMAGE_DRAG_BTN			
		};
		
		enum 
		{
			MOVECOMMAND_SCROLLBTN_WIDTH = 15,
			MOVECOMMAND_SCROLLBTN_HEIGHT = 30,
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH = 7,
			MOVECOMMAND_SCROLLBAR_TOP_HEIGHT = 3,
			MOVECOMMAND_SCROLLBAR_MIDDLE_WIDTH = 7,
			MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT = 15,

			// 밑에 값은 기본 초기값으로만 사용하고 실제 적용은 m_iTextLine 변수로 이루어 진다.
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
			MOVECOMMAND_MAX_RENDER_TEXTLINE = 16,		// 이동명령 정보창에 보여지는 텍스트갯수를 여기서 정한다.(Default: 16)
#else // CSK_MOD_MOVE_COMMAND_WINDOW
			MOVECOMMAND_MAX_RENDER_TEXTLINE = 31,		// 이동명령 정보창에 보여지는 텍스트갯수를 여기서 정한다.(Default: 31)
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
			
		};

		enum MOVECOMMAND_MOUSE_EVENT
		{
			MOVECOMMAND_MOUSEBTN_NORMAL = 0,
			MOVECOMMAND_MOUSEBTN_OVER,
			MOVECOMMAND_MOUSEBTN_CLICKED,
		};

#ifdef CSK_MOD_PROTECT_AUTO_V1
	protected:
#else // CSK_MOD_PROTECT_AUTO_V1
	private:
#endif // CSK_MOD_PROTECT_AUTO_V1
	
//$$AUTO_BUILD_LINE_ SHUFFLE_BEGIN
		CNewUIManager*				m_pNewUIMng;
		POINT						m_Pos;

		int							m_iRealFontHeight;
			
		std::list<CMoveCommandData::MOVEINFODATA*>	m_listMoveInfoData;
		POINT						m_StartUISubjectName;	// UI제목 위치
		POINT						m_StartMapNamePos;		// 맵 이름 시작 위치
		POINT						m_MapNameUISize;		// 맵 이동명령UI SIZE
#ifdef ASG_ADD_GENS_SYSTEM
		POINT						m_StrifePos;			// 분쟁지역
#endif	// ASG_ADD_GENS_SYSTEM
		POINT						m_MapNamePos;
		POINT						m_ReqLevelPos;
		POINT						m_ReqZenPos;
		int							m_iSelectedMapName;		// 선택된 맵인덱스

		POINT						m_ScrollBarPos;
		POINT						m_ScrollBtnStartPos;
		POINT						m_ScrollBtnPos;

		int							m_iScrollBarHeightPixel;	// 스크롤바 길이
		int							m_iRenderStartTextIndex;	// UI에 렌더되는 처음 Text의 인덱스
		int							m_iRenderEndTextIndex;		// UI에 렌더되는 마지막 Text의 인덱스
		int							m_iSelectedTextIndex;		// 현재 선택된 Text
		int							m_iScrollBtnInterval;		// 스크롤바 움직이는 간격
		int							m_iScrollBarMiddleNum;				// 스크롤바 Middle 갯수
		int							m_iScrollBarMiddleRemainderPixel;	// 스크롤바 Middle갯수대로 렌더후 남은 픽셀
		int							m_iNumPage;					// 전체페이지
		int							m_iCurPage;					// 현재페이지
//$$AUTO_BUILD_LINE_ SHUFFLE_END
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
//$$AUTO_BUILD_LINE_ SHUFFLE_BEGIN
		int							m_iTotalMoveScrBtnPixel;			// 스크롤버튼의 전체 픽셀값
		int							m_iRemainMoveScrBtnPixel;		// 스크롤버튼의 이동해야할 나머지 픽셀값
		int							m_icurMoveScrBtnPixelperStep;	// 스크롤버튼의 현재 이동 픽셀값
		int							m_iMaxMoveScrBtnPixelperStep;	// 스크롤버튼의 이동하는 최대픽셀값
		int							m_iMinMoveScrBtnPixelperStep;	// 스크롤버튼의 이동하는 최소픽셀값
		int							m_iTotalMoveScrBtnperStep;		// 스크롤버튼의 이동해야할 전체 칸의 갯수
		int							m_iRemainMoveScrBtnperStep;		// 스크롤버튼의 이동해야할 나머지 칸의 갯수
		int							m_iTotalNumMaxMoveScrBtnperStep;	// 스크롤버튼의 최대이동칸의 전체 갯수
		int							m_iTotalNumMinMoveScrBtnperStep;	// 스크롤버튼의 최소이동칸의 전체 갯수
		int							m_iAcumMoveMouseScrollPixel;		// 마우스 스크롤값 누적(한칸 이동할때 쓰인다. ScrollUp/ScrollDown)
//$$AUTO_BUILD_LINE_ SHUFFLE_END
#else KJH_FIX_MOVECOMMAND_WINDOW_SIZE		// 정리할때 지워야 하는소스
		int							m_iMoveSCBtnPerPage;		// 페이지당 스크롤버튼 이동값
		int							m_iNumMoveStep;				// 스크롤 움직이는 단계
		int							m_iHeightByMoveStep;		// 스크롤 움직이는 단계 단위길이
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE	// 정리할때 지워야 하는소스
//$$AUTO_BUILD_LINE_ SHUFFLE_BEGIN
		int							m_iMousePosY;
		int							m_iScrollBtnMouseEvent;				// 스크롤 버튼 마우스 클릭
		bool						m_bScrollBtnActive;					// 스크롤버튼 활성화
//$$AUTO_BUILD_LINE_ SHUFFLE_END
		

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
//$$AUTO_BUILD_LINE_ SHUFFLE_BEGIN
		int							m_iPosY_Random;			// Y위치 랜덤값
		int							m_iPosY_Random_Min;		// Y위치 랜덤 최소값 
		int							m_iPosY_Random_Range;	// Y위치 랜덤값 범위
		POINT						m_Pos_Start;			// 창 생성 시작 위치 Create함수 호출 할 때 입력 받은 값
		int							m_iWindowOpenCount;		// 창 연 횟수
		int							m_iWorldIndex;			// 맵 인덱스
		bool						m_bCastleOwner;			// 수성측 길드나 연합이면 true 아니면 false
//$$AUTO_BUILD_LINE_ SHUFFLE_END
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
#ifdef CSK_MOD_PROTECT_AUTO_V1
		int							m_iTextLine;			// 화면에 표시될 텍스트 라인수
#ifndef CSK_MOD_REMOVE_AUTO_V1_FLAG		// 정리할 때 지워야 하는 소스
		bool						m_bNewVersion;			// 맵창 새로운 버전인가
#endif //! CSK_MOD_REMOVE_AUTO_V1_FLAG	// 정리할 때 지워야 하는 소스
		int							m_iTextAlpha;			// 텍스트 알파값
		float						m_fBackgroundAlpha;		// 배경 알파값
		int							m_iTopSpace;			// 상단 여백
#endif // CSK_MOD_PROTECT_AUTO_V1	
#ifdef YDG_MOD_PROTECT_AUTO_V3
		int							m_iWheelCounter;		// 먹통 처리용 휠 카운터
#endif	// YDG_MOD_PROTECT_AUTO_V3
#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
		DWORD						m_dwMoveCommandKey;		// 이동 명령 전송시 키 값
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

	public:
		CNewUIMoveCommandWindow();
		virtual ~CNewUIMoveCommandWindow();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		bool BtnProcess();
		
		float GetLayerDepth();	//. 6.4f
		
		virtual void OpenningProcess();
		void ClosingProcess();

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
		void ResetWindowOpenCount();
		void SetCastleOwner(bool bOwner);
		bool IsCastleOwner();
#endif // CSK_MOD_MOVE_COMMAND_WINDOW

#ifndef CSK_MOD_REMOVE_AUTO_V1_FLAG  // 정리할 때 지워야 하는 소스
		void SetNewVersion(bool bNew);
		bool IsNewVersion();
#endif //! CSK_MOD_REMOVE_AUTO_V1_FLAG // 정리할 때 지워야 하는 소스

#ifdef CSK_LUCKY_SEAL
		bool IsLuckySealBuff();
		bool IsMapMove( const string& src );
#endif //CSK_LUCKY_SEAL
	
#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
		void SetMoveCommandKey(DWORD dwKey);	// 초기값 설정
		DWORD GetMoveCommandKey();				// 초기값을 바탕으로 키값을 만들어낸다 [주의!] SendRequestMoveMap 한번 보낼때 한번만 호출되어야 한다!
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

#ifdef LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
		BOOL IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const;
#endif //LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		int GetMapIndexFromMovereq(const char *pszMapName);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

#ifdef CSK_MOD_PROTECT_AUTO_V1
	protected:
#else // CSK_MOD_PROTECT_AUTO_V1
	private:
#endif // CSK_MOD_PROTECT_AUTO_V1
#ifdef ASG_ADD_GENS_SYSTEM
		void SetStrifeMap();
#endif	// ASG_ADD_GENS_SYSTEM
		void SettingCanMoveMap();
		void RenderFrame();
		//void MoveScrollBtn(int iMoveValue);
		void UpdateScrolling();
		void ScrollUp(int iMoveValue);
		void ScrollDown(int iMoveValue);
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
		// bSign : true(DownScroll,+), false(UpScroll,-)
		void RecursiveCalcScroll(IN int piScrollValue, OUT int* piMovePixel, bool bSign = true);
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
		void LoadImages();
		void UnloadImages();
	};
	
#ifdef YDG_MOD_PROTECT_AUTO_V4
	class CMoveCommandWindowEncrypt
	{
	public:
		CMoveCommandWindowEncrypt();
		virtual ~CMoveCommandWindowEncrypt();
		static void Enable(BOOL bFlag);	// CMoveCommandWindow가 초기화 될때 한번만 호출
#ifdef YDG_MOD_PROTECT_AUTO_V4_R2
		static int Read(int * piValue);
		static LONG Read(LONG * plValue);
		static void Write(int * piValue, int iValue);
		static void Write(LONG * plValue, LONG lValue);
#endif	// YDG_MOD_PROTECT_AUTO_V4_R2

	protected:
		static void Encrypt(int * piValue);
		static void Decrypt(int * piValue);
		static void Encrypt(POINT * pValue);
		static void Decrypt(POINT * pValue);
		static void EncryptWindowPos();
		static void DecryptWindowPos();

	protected:
		static DWORD m_dwEncryptSeed;
		static int m_iEncryptCounter;	// 0: 암호화, 1+:암호 해제상태
	};
#endif	// YDG_MOD_PROTECT_AUTO_V4
};

#ifdef CSK_MOD_PROTECT_AUTO_V1
class CNewUIMoveCommandWindowNew : public SEASON3B::CNewUIMoveCommandWindow
{
public:
	CNewUIMoveCommandWindowNew();
	virtual ~CNewUIMoveCommandWindowNew();

	virtual void OpenningProcess();
};
#endif // CSK_MOD_PROTECT_AUTO_V1

#endif	// YDG_ADD_ENC_MOVE_COMMAND_WINDOW

#endif // !defined(AFX_NEWUIMOVECOMMANDWINDOW_H__E6ABC928_0A79_41CD_8046_5ED052B1985A__INCLUDED_)

