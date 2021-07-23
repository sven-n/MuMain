
#ifndef _NEWUICUSOMMESSAGEBOX_H_
#define _NEWUICUSOMMESSAGEBOX_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIMessageBox.h"
#include "NewUICommonMessageBox.h"
#include "UIControls.h"

namespace SEASON3B
{
	enum
	{
		INPUTBOX_TYPE_NUMBER,
		INPUTBOX_TYPE_TEXT,	
	};

	enum KEYPAD_TYPE
	{
		KEYPAD_TYPE_MOVE = 1,
		KEYPAD_TYPE_UNLOCK = 2,
		KEYPAD_TYPE_LOCK_FIRST = 3,
		KEYPAD_TYPE_LOCK_SECOND = 4,
		KEYPAD_TYPE_LOCK_FINAL = 5,
	};

	enum
	{
		INPUTBOX_WIDTH = 50,
		INPUTBOX_HEIGHT = 12,

		INPUTBOX_TEXTLIMIT = 8,
	};

	//////////////////////////////////////////////////////////////////////////

	// 숫자나 문자 입력처리 되는 메세지박스
	class CNewUITextInputMsgBox : public CNewUIMessageBoxBase
	{
		enum
		{
			INPUTBOX_TOP_BLANK = 10,
		};

		typedef struct _MSGBOX_TEXTDATA 
		{
			unicode::t_string strMsg;
			DWORD dwColor;
			BYTE byFontType;

			_MSGBOX_TEXTDATA()
			{
				strMsg = "";
				dwColor = 0xffffffff;
				byFontType = MSGBOX_FONT_NORMAL;
			}
		} MSGBOX_TEXTDATA;

		typedef std::vector<MSGBOX_TEXTDATA*> type_vector_msgdata;
		typedef unicode::t_string type_string;

	public:
		CNewUITextInputMsgBox();
		virtual ~CNewUITextInputMsgBox();

		bool Create(DWORD dwMsgBoxType, DWORD dwInputType, int iInputBoxWidth = 100, int iInputBoxHeight = 14, int iLimitText = 256, bool bIsPassword = false);
		void Release();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

		bool Update();
		bool Render();

		DWORD GetMsgBoxType();

		// InputBox 랩핑 함수
		void GetInputBoxText(unicode::t_char* strText);
		void SetInputBoxOption(int iOption);
		void SetInputBoxPosition(int x, int y);
		void SetInputBoxSize(int width, int height);

	private:
		int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);
		void SetButtonInfo();
		void AddButtonBlank(int iAddLine);
		void RenderTexts();
		void RenderButtons();

		DWORD m_dwMsgBoxType;
		DWORD m_dwInputType;

		CUITextInputBox* m_pInputBox;
		type_vector_msgdata m_MsgTextList;

		CNewUIMessageBoxButton m_BtnOk;
		CNewUIMessageBoxButton m_BtnCancel;

#ifdef LDK_MOD_GLOBAL_STORAGELOCK_CHANGE
	public:
		void SetPassword(WORD password) { m_password = password; }
		WORD GetPassword() { return m_password; }

	private:
		WORD m_password;
#endif //LDK_MOD_GLOBAL_STORAGELOCK_CHANGE
	};

	//////////////////////////////////////////////////////////////////////////
	
	class CNewUIKeyPadButton : public CNewUIMessageBoxButton
	{
	public:
		void Render();
	};

	class CNewUIDeleteKeyPadButton : public CNewUIMessageBoxButton
	{
	public:
		void Render();
	};

	//////////////////////////////////////////////////////////////////////////
	// 비밀번호나 주민등록번호 입력받는 키패드 메세지박스
	class CNewUIKeyPadMsgBox : public CNewUIMessageBoxBase
	{	
		enum
		{
			MSGBOX_MIDDLE_FRAME_NUM = 9,
			KEYPAD_INPUT_NUM = 2,
			KEYPAD_WIDTH = 32,
			KEYPAD_HEIGHT = 32,
		};

		typedef struct _MSGBOX_TEXTDATA 
		{
			unicode::t_string strMsg;
			DWORD dwColor;
			BYTE byFontType;

			_MSGBOX_TEXTDATA()
			{
				strMsg = "";
				dwColor = 0xffffffff;
				byFontType = MSGBOX_FONT_NORMAL;
			}
		} MSGBOX_TEXTDATA;

		typedef std::vector<MSGBOX_TEXTDATA*> type_vector_msgdata;
		typedef unicode::t_string type_string;

	public:
		CNewUIKeyPadMsgBox();
		virtual ~CNewUIKeyPadMsgBox();

		bool Create(DWORD dwType, int iInputLImit = 4);
		void Release();

		bool Update();
		bool Render();

		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

		int GetInputLimit();
		int GetInputSize();
		void ClearInput();
		// 유니코드 아니여야 함!
		const char* GetInputText();
		void SetCheckInputText(const char* strInput);
		bool IsCheckInput();
		// 입력된 번호들이 모두 같은 번호인가?
		bool IsAllSameNumber();

		void SetStoragePassword(WORD wPassword);
		WORD GetStoragePassword();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT DeleteBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT KeyPadBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT Close(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	
	private:
		void SetButtonInfo();

		void KeyPadInput(int iInput);
		void DeleteKeyPadInput();
		
		void RenderFrame();
		void RenderKeyPadInput();
		void RenderTexts();
		void RenderButtons();

		DWORD m_dwType;

		// 키패드 정보들
		int m_iInputLimit;
		int m_iKeyPadMapping[MAX_KEYPADINPUT];

		// 키패드 입력정보들(유니코드 아니여야 함!)
		// 서버와 주고 받는 데이타 타입이 문자 타입임 ㅡㅡ;
#ifdef LDK_MOD_PASSWORD_LENGTH_20
		//글로벌 포털 
		char m_strKeyPadInput[MAX_PASSWORD_SIZE+ 1];
#else //LDK_MOD_PASSWORD_LENGTH_20
		char m_strKeyPadInput[MAX_KEYPADINPUT + 1];
#endif //LDK_MOD_PASSWORD_LENGTH_20
		char m_strCheckKeyPadInput[MAX_KEYPADINPUT + 1];

		// 비밀번호 임시로 저장해 놓는 변수
		WORD m_wStoragePassword;

		// 텍스트들
		type_vector_msgdata m_MsgTextList;

		// 버튼들
		CNewUIKeyPadButton m_BtnKeyPad[MAX_KEYPADINPUT];
		CNewUIDeleteKeyPadButton m_BtnDeleteKeyPad;
		CNewUIMessageBoxButton m_BtnOk;
		CNewUIMessageBoxButton m_BtnCancel;
	};

	//////////////////////////////////////////////////////////////////////////

	// 열매 사용에서 생성, 감소, 취소 버튼있는 메세지 박스
	class CUseFruitCheckMsgBox : public CNewUIMessageBoxBase, public INewUI3DRenderObj
	{
		enum
		{
			MSGBOX_TEXT_MAXWIDTH_3DITEM = 120,
			MSGBOX_TEXT_LEFT_BLANK_3DITEM = 60,
			MSGBOX_3DITEM_WIDTH = 40,
			MSGBOX_3DITEM_HEIGHT = 40,	
		};
	public:	
		CUseFruitCheckMsgBox();
		virtual ~CUseFruitCheckMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		void Set3DItem(ITEM* pItem);

		bool Update();
		bool Render();
		void Render3D();

		bool IsVisible() const;

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT AddBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT MinusBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
		void SetAddCallbackFunc();
		void SetButtonInfo();
		
		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		ITEM m_Item;

		type_vector_msgdata m_MsgDataList;

		// button
		CNewUIMessageBoxButton m_BtnAdd;
		CNewUIMessageBoxButton m_BtnMinus;
		CNewUIMessageBoxButton m_BtnCancel;
	};

	//////////////////////////////////////////////////////////////////////////

	// 보석 통합 해체 메세지박스
	class CGemIntegrationMsgBox : public CNewUIMessageBoxBase
	{
		enum
		{
			MIDDLE_COUNT = 5,
			BTN_TOP_BLANK = 60,
			BTN_GAP = 40,
		};
	public:
		CGemIntegrationMsgBox();
		virtual ~CGemIntegrationMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT UnityBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT DisjointBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// texts
		type_vector_msgdata m_MsgDataList;
		// button
		CNewUIMessageBoxButton m_BtnUnity;
		CNewUIMessageBoxButton m_BtnDisjoint;
		CNewUIMessageBoxButton m_BtnCancel;
	};
	
	// 보석 통합 메세지박스
	class CGemIntegrationUnityMsgBox : public CNewUIMessageBoxBase
	{
		enum
		{
			STATE_BASIC,
		};
		enum
		{
			MIDDLE_COUNT = 10,
			BTN_TOP_BLANK = 60,
			BTN_GAP = 40,
		};
	public:
		CGemIntegrationUnityMsgBox();
		virtual ~CGemIntegrationUnityMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();
		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT BlessingBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT SoulBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT TenBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT TwentyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ThirtyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
		static CALLBACK_RESULT SelectMixBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	private:
		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// texts
		type_vector_msgdata m_MsgDataList;
		// button

	#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
		void SetText( void );
		void ResetWndSize( int _nType );
		std::vector<CNewUIMessageBoxButton>	m_cJewelButton;
		std::vector<CNewUIMessageBoxButton>	m_cMixButton;
		int									m_nMiddleCount;
	#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX

		CNewUIMessageBoxButton m_BtnBlessing;
		CNewUIMessageBoxButton m_BtnSoul;
		CNewUIMessageBoxButton m_BtnTen;
		CNewUIMessageBoxButton m_BtnTwenty;
		CNewUIMessageBoxButton m_BtnThirty;
		CNewUIMessageBoxButton m_BtnCancel;
	};

	// 보석 해체 메세지박스
	class CGemIntegrationDisjointMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CGemIntegrationDisjointMsgBox();
		virtual ~CGemIntegrationDisjointMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT BlessingBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT SoulBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT DisjointBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
		void SetAddCallbackFunc();
		void SetButtonInfo();
		void ChangeMiddleFrameSmall();
		void ChangeMiddleFrameBig();

		void RenderFrame();
		void RenderTexts();
		void RenderGemList();
		void RenderButtons();

		int m_iMiddleFrameCount;

		// texts
		type_vector_msgdata m_MsgDataList;
		// button
		CNewUIMessageBoxButton m_BtnBlessing;
		CNewUIMessageBoxButton m_BtnSoul;
		CNewUIMessageBoxButton m_BtnDisjoint;
		CNewUIMessageBoxButton m_BtnCancel;
	};

	// 시스템 메뉴 메세지박스
	class CSystemMenuMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CSystemMenuMsgBox();
		virtual ~CSystemMenuMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();
		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT GameOverBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ChooseServerBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ChooseCharacterBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OptionBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		
	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();
		void RenderFrame();
		void RenderButtons();

		// button
		CNewUIMessageBoxButton m_BtnGameOver;			// 게임종료
		CNewUIMessageBoxButton m_BtnChooseServer;		// 서버선택
		CNewUIMessageBoxButton m_BtnChooseCharacter;	// 캐릭터선택
		CNewUIMessageBoxButton m_BtnOption;				// 옵션
		CNewUIMessageBoxButton m_BtnCancel;				// 취소
	};

	// 블러드캐슬 결과 메세지박스
	class CBloodCastleResultMsgBox : public CNewUIMessageBoxBase
	{
		enum
		{
			MIDDLE_COUNT = 6,
		};
	public:
		CBloodCastleResultMsgBox();
		virtual ~CBloodCastleResultMsgBox();

		bool Create(float fPriority = 3.f);

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void RenderFrame();

		CNewUIMessageBoxButton m_BtnOk;
	};

	// 악마의광장 랭킹 메세지박스
	class CDevilSquareRankMsgBox : public CNewUIMessageBoxBase
	{
		enum
		{
			MIDDLE_COUNT1 = 11,
			MIDDLE_COUNT2 = 3,

		};
	public:
		CDevilSquareRankMsgBox();
		virtual ~CDevilSquareRankMsgBox();

		bool Create(float fPriority = 3.f);

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void RenderFrame();

		CNewUIMessageBoxButton m_BtnOk;
	};

	class CChaosCastleResultMsgBox : public CNewUIMessageBoxBase
	{
		enum
		{
			MIDDLE_COUNT = 6,
		};
	public:
		CChaosCastleResultMsgBox();
		virtual ~CChaosCastleResultMsgBox();

		bool Create(float fPriority = 3.f);

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void RenderFrame();

		CNewUIMessageBoxButton m_BtnOk;
	};

	class CChaosMixMenuMsgBox : public CNewUIMessageBoxBase
	{
		enum
		{
			MIDDLE_COUNT = 13,
		};
	public:
		CChaosMixMenuMsgBox();
		virtual ~CChaosMixMenuMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT GeneralMixBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ChaosMixBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT Mix380BtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// buttons
		CNewUIMessageBoxButton m_BtnGeneralMix;
		CNewUIMessageBoxButton m_BtnChaosMix;
		CNewUIMessageBoxButton m_BtnMix380;
		CNewUIMessageBoxButton m_BtnCancel;
	};


	// 조련사 메인 메뉴
	class CTrainerMenuMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CTrainerMenuMsgBox();
		~CTrainerMenuMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT RecoverBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ReviveBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// buttons
		CNewUIMessageBoxButton m_BtnRecover;
		CNewUIMessageBoxButton m_BtnRevive;
		CNewUIMessageBoxButton m_BtnExit;

		int m_iMiddleCount;
	};

	// 조련사 회복 메뉴
	class CTrainerRecoverMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CTrainerRecoverMsgBox();
		~CTrainerRecoverMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT RecoverDarkSpiritrBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT RecoverDarkHorseBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// buttons
		CNewUIMessageBoxButton m_BtnRecoverDarkSpirit;
		CNewUIMessageBoxButton m_BtnRecoverDarkHorse;
		CNewUIMessageBoxButton m_BtnExit;

		int m_iMiddleCount;
	};


	// 엘피스 메뉴 (제련석)
	class CElpisMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CElpisMsgBox();
		~CElpisMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT AboutRefinaryBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT AboutJewelOfHarmonyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT RefineBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

		void SetMessageType(int iMessageType) { m_iMessageType = iMessageType; }

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// buttons
		CNewUIMessageBoxButton m_BtnAboutRefinary;
		CNewUIMessageBoxButton m_BtnAboutJewelOfHarmony;
		CNewUIMessageBoxButton m_BtnRefine;
		CNewUIMessageBoxButton m_BtnExit;

		int m_iMiddleCount;
		int m_iMessageType;
	};
	
	class CDialogMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CDialogMsgBox();
		~CDialogMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT EndBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	
		bool Update();
		bool Render();

	private:
		void SetAddCallbackFunc();
		int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);

		void SetButtonInfo();
		void AddButtonBlank(int iAddLine);

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// text
		type_vector_msgdata m_MsgDataList;
		// button
		CNewUIMessageBoxButton m_BtnEnd;
	};
	
	// Progress바가 있는 메세지박스
	class CProgressMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CProgressMsgBox();
		~CProgressMsgBox();

		bool Create(DWORD dwElapseTime = 3000, float fPriority = 3.f);
		void Release();

		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
		void SetElapseTime(DWORD dwElapseTime);

		bool Update();
		bool Render();

		static CALLBACK_RESULT ClosingProcess(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);

		void RenderFrame();
		void RenderTexts();
		void RenderProgress();

	private:
		type_vector_msgdata m_MsgDataList;
		
		DWORD m_dwStartTime;		// 시작된 시간
		DWORD m_dwEndTime;			// 종료될 시간
		DWORD m_dwElapseTime;		// 경과 시간
	};

	// 환영사원 전용 Progress바 메세지박스
	class CCursedTempleProgressMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CCursedTempleProgressMsgBox();
		~CCursedTempleProgressMsgBox();

		bool Create(DWORD dwElapseTime = 3000, float fPriority = 3.f);
		void Release();

		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

		bool Update();
		bool Render();

		void SetNpcIndex(DWORD dwIndex);
		DWORD GetNpcIndex();

		static CALLBACK_RESULT ClosingProcess(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CompleteProcess(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);

		void RenderFrame();
		void RenderTexts();
		void RenderProgress();
		
		bool CheckHeroAction();

	private:
		type_vector_msgdata m_MsgDataList;
		
		DWORD m_dwStartTime;		// 시작된 시간
		DWORD m_dwEndTime;			// 종료될 시간
		DWORD m_dwElapseTime;		// 경과 시간

		DWORD m_dwNpcIndex;
	};

	// 결투신청 메세지박스
	class CDuelMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CDuelMsgBox();
		~CDuelMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButton();
	
		// button
		CNewUIMessageBoxButton m_BtnOk;
		CNewUIMessageBoxButton m_BtnCancel;
	};
#ifdef YDG_ADD_NEW_DUEL_UI
	// 결투결과 메세지박스
	class CDuelResultMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CDuelResultMsgBox();
		~CDuelResultMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

		void SetIDs(char * pszWinnerID, char * pszLoserID);

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButton();
	
		// button
		CNewUIMessageBoxButton m_BtnOk;
		char m_szWinnerID[24];
		char m_szLoserID[24];
	};
#endif	// YDG_ADD_NEW_DUEL_UI

	class CGuild_ToPerson_Position : public CNewUIMessageBoxBase
	{
		enum
		{
			STATE_BASIC,
		};
		enum
		{
			MIDDLE_COUNT = 10,
			BTN_TOP_BLANK = 60,
			BTN_GAP = 40,
		};
		enum GUILD_STATUS
		{
			G_NONE			= (BYTE)-1,
			G_PERSON		= 0,
			G_MASTER		= 128,
			G_SUB_MASTER	= 64,
			G_BATTLE_MASTER	= 32
		};

	public:
		CGuild_ToPerson_Position();
		~CGuild_ToPerson_Position();

		bool Create(float fPriority = 3.f);
		void Release();
		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT BlessingBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT SoulBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// texts
		type_vector_msgdata m_MsgDataList;
		// button
		CNewUIMessageBoxButton m_BtnBlessing;
		CNewUIMessageBoxButton m_BtnSoul;
		CNewUIMessageBoxButton m_BtnOk;
		CNewUIMessageBoxButton m_BtnCancel;
	};	


#ifdef CSK_EVENT_CHERRYBLOSSOM
	class CCherryBlossomMsgBox : public CNewUIMessageBoxBase
	{
	public:	
		CCherryBlossomMsgBox();
		~CCherryBlossomMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();
		
		bool Update();
		bool Render();
		
		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT WhiteCBBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT RedCBBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT GodCBBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();
		
		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// buttons
		CNewUIMessageBoxButton m_BtnWhiteCB;
		CNewUIMessageBoxButton m_BtnRedCB;
		CNewUIMessageBoxButton m_BtnGoldCB;
		CNewUIMessageBoxButton m_BtnExit;

		int m_iMiddleCount;
	};
#endif // CSK_EVENT_CHERRYBLOSSOM
	
#ifdef LEM_ADD_LUCKYITEM
	class CLuckyTradeMenuMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CLuckyTradeMenuMsgBox();
		~CLuckyTradeMenuMsgBox();
		
		bool Create(float fPriority = 3.f);
		void Release();
		
		bool Update();
		bool Render();
		
		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT LuckyItemTradeBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT LuckyItemRefineryBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		
	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();
		
		void RenderFrame();
		void RenderTexts();
		void RenderButtons();
		
		// buttons
		CNewUIMessageBoxButton m_BtnTrade;
		CNewUIMessageBoxButton m_BtnRefinery;
		CNewUIMessageBoxButton m_BtnExit;
		
		int m_iMiddleCount;
	};
#endif // LEM_ADD_LUCKYITEM
#ifdef ADD_SOCKET_MIX
	// 시드마스터 메인 메뉴
	class CSeedMasterMenuMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CSeedMasterMenuMsgBox();
		~CSeedMasterMenuMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExtractSeedBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT SeedSphereBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// buttons
		CNewUIMessageBoxButton m_BtnExtractSeed;
		CNewUIMessageBoxButton m_BtnSeedSphere;
		CNewUIMessageBoxButton m_BtnExit;

		int m_iMiddleCount;
	};

	// 시드연구가 메인 메뉴
	class CSeedInvestigatorMenuMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CSeedInvestigatorMenuMsgBox();
		~CSeedInvestigatorMenuMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT AttachSocketBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT DetachSocketBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// buttons
		CNewUIMessageBoxButton m_BtnAttachSocket;
		CNewUIMessageBoxButton m_BtnDetachSocket;
		CNewUIMessageBoxButton m_BtnExit;

		int m_iMiddleCount;
	};
#endif	// ADD_SOCKET_MIX


#ifdef PSW_ADD_RESET_CHARACTER_POINT
	class CResetCharacterPointMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CResetCharacterPointMsgBox();
		~CResetCharacterPointMsgBox();
		
		bool Create(float fPriority = 3.f);
		void Release();
		
		bool Update();
		bool Render();
		
		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ResetCharacterPointBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		
	private:
		void SetButtonInfo();
		bool isCharacterEquipmentItem();
		void SetAddCallbackFunc();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();
		
	private:
		CNewUIMessageBoxButton m_ResetCharacterPointBtn;
		CNewUIMessageBoxButton m_BtnExit;
		int m_iMiddleCount;
	};
#endif	// PSW_ADD_RESET_CHARACTER_POINT
	
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	// 델가도(7주년기념이벤트) 메인메뉴
	class CDelgardoMainMenuMsgBox : public CNewUIMessageBoxBase
	{
	public:
		CDelgardoMainMenuMsgBox();
		~CDelgardoMainMenuMsgBox();

		bool Create(float fPriority = 3.f);
		void Release();

		bool Update();
		bool Render();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT RegBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExchangeBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		void SetAddCallbackFunc();
		void SetButtonInfo();

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		// buttons
		CNewUIMessageBoxButton m_BtnReg;
		CNewUIMessageBoxButton m_BtnExchange;
		CNewUIMessageBoxButton m_BtnExit;

		int m_iMiddleCount;
	};
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008

	
	//////////////////////////////////////////////////////////////////////////
	///////////////////////// Custom MsgBox Layout ///////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 거래 젠 입력 처리 메세지박스
	class CTradeZenMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		
		static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner);
	};

	// 젠 입금 처리 메세지박스
	class CZenReceiptMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	private:
		static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	//////////////////////////////////////////////////////////////////////////

	// 젠 출금 처리 메세지박스
	class CZenPaymentMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	private:
		static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 개인상점 아이템 판매가격 입력 메세지박스
	class CPersonalShopItemValueMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	private:
		static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 개인 상점명 입력 메세지박스
	class CPersonalShopNameMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
	{
		enum
		{
			INPUT_WIDTH = 130,
			INPUT_HEIGHT = 12,
			INPUT_TEXTLIMIT = 28,
		};
	public:
		bool SetLayout();
		static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);	
	};

	class CCastleWithdrawMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	//////////////////////////////////////////////////////////////////////////
#ifdef LDK_MOD_GLOBAL_STORAGELOCK_CHANGE
	// 글로벌 포털용 창고 비번 확인용 패스워드창
	class CStorageLockMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 창고 잠금해제
	class CStorageUnlockMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);		
	};

#endif //LDK_MOD_GLOBAL_STORAGELOCK_CHANGE

	// 출금할 때 비밀번호 키패드 메세지박스
	// 창고에서 인벤토리로 아이템 옮길때 비밀번호 물어보는 키패드 메세지 박스
	class CPasswordKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);		
	};

	// 창고 잠금 상태 키패드 메세지박스
	class CStorageLockKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);		
	};

	// 창고 잠금 확인 키패드 메세지박스
	class CStorageLockCheckKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 창고 잠금 최종 주민번호 키패드 메세지박스
	class CStorageLockFinalKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 창고 잠금해제 상태 키패드 메세지박스
	class CStorageUnlockKeyPadMsgBoxLayout : public TMsgBoxLayout<CNewUIKeyPadMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);		
	};

	// 열매 사용에서 생성, 감소, 취소 버튼있는 메세지 박스
	class CUseFruitCheckMsgBoxLayout : public TMsgBoxLayout<CUseFruitCheckMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 보석 통합 해체 메세지박스
	class CGemIntegrationMsgBoxLayout : public TMsgBoxLayout<CGemIntegrationMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 보석 통합 메세지박스
	class CGemIntegrationUnityMsgBoxLayout : public TMsgBoxLayout<CGemIntegrationUnityMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 보석 해체 메세지박스
	class CGemIntegrationDisjointMsgBoxLayout : public TMsgBoxLayout<CGemIntegrationDisjointMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 블러드캐슬 결과 메세지박스
	class CBloodCastleResultMsgBoxLayout : public TMsgBoxLayout<CBloodCastleResultMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 악마의 광장 랭킹 메세지박스
	class CDevilSquareRankMsgBoxLayout : public TMsgBoxLayout<CDevilSquareRankMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 카오스 캐슬 결과 메세지박스
	class CChaosCastleResultMsgBoxLayout : public TMsgBoxLayout<CChaosCastleResultMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 노리아에 있는 카오스 조합 메뉴 메세지박스
	class CChaosMixMenuMsgBoxLayout : public TMsgBoxLayout<CChaosMixMenuMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 조련사 메뉴
	class CTrainerMenuMsgBoxLayout : public TMsgBoxLayout<CTrainerMenuMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 조련사 회복 메뉴
	class CTrainerRecoverMsgBoxLayout : public TMsgBoxLayout<CTrainerRecoverMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 엘피스 제련 메뉴
	class CElpisMsgBoxLayout : public TMsgBoxLayout<CElpisMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 대화 메세지박스
	class CDialogMsgBoxLayout : public TMsgBoxLayout<CDialogMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 왕관 스위치 땜
	class CCrownSwitchPopLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 왕관 스위치 누름
	class CCrownSwitchPushLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 왕관 스위치 다른사람이 누르고 있음
	class CCrownSwitchOtherPushLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 직인 등록 시작
	class CSealRegisterStartLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 직인 등록 성공
	class CSealRegisterSuccessLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 직인 등록 실패
	class CSealRegisterFailLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 다른 사람이 직인 등록 중
	class CSealRegisterOtherLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 다른 진영이 직인 등록 중
	class CSealRegisterOtherCampLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 왕관 방어막 해제
	class CCrownDefenseRemoveLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 왕관 방어막 생성
	class CCrownDefenseCreateLayout : public TMsgBoxLayout<CProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	// 환영사원 성물 획득 프로그레스바
	class CCursedTempleHolicItemGetLayout : public TMsgBoxLayout<CCursedTempleProgressMsgBox>
	{
	public:
		bool SetLayout();
	};

	class CCursedTempleHolicItemSaveLayout : public TMsgBoxLayout<CCursedTempleProgressMsgBox>
	{
	public:
		bool SetLayout();
	};
	
	// 시스템 메뉴 메세지박스
	class CSystemMenuMsgBoxLayout : public TMsgBoxLayout<CSystemMenuMsgBox>
	{
	public:
		bool SetLayout();
	};

	//길드해체시 주민등록 입력
	class CGuildBreakPasswordMsgBoxLayout : public TMsgBoxLayout<CNewUITextInputMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

	private:
		static CALLBACK_RESULT ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CGuild_ToPerson_PositionLayout : public TMsgBoxLayout<CGuild_ToPerson_Position>
	{
	public:
		bool SetLayout();
	};

	// 결투신청 메세지박스
	class CDuelMsgBoxLayout : public TMsgBoxLayout<CDuelMsgBox>
	{
	public:
		bool SetLayout();
	};
#ifdef YDG_ADD_NEW_DUEL_UI
	// 결투결과 메세지박스
	class CDuelResultMsgBoxLayout : public TMsgBoxLayout<CDuelResultMsgBox>
	{
	public:
		bool SetLayout();
	};
#endif	// YDG_ADD_NEW_DUEL_UI

#ifdef CSK_EVENT_CHERRYBLOSSOM
	class CCherryBlossomMsgBoxLayout : public TMsgBoxLayout<CCherryBlossomMsgBox>
	{
	public:
		bool SetLayout();
	};
#endif // CSK_EVENT_CHERRYBLOSSOM

#ifdef ADD_SOCKET_MIX
	// 시드마스터 메뉴
	class CSeedMasterMenuMsgBoxLayout : public TMsgBoxLayout<CSeedMasterMenuMsgBox>
	{
	public:
		bool SetLayout();
	};
	// 시드연구가 메뉴
	class CSeedInvestigatorMenuMsgBoxLayout : public TMsgBoxLayout<CSeedInvestigatorMenuMsgBox>
	{
	public:
		bool SetLayout();
	};
#endif	// ADD_SOCKET_MIX
	

#ifdef PSW_ADD_RESET_CHARACTER_POINT
	class CResetCharacterPointMsgBoxLayout : public TMsgBoxLayout<CResetCharacterPointMsgBox>
	{
	public:
		bool SetLayout();
	};
#endif //PSW_ADD_RESET_CHARACTER_POINT

#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	// 델가도(7주년기념이벤트) 메인메뉴
	class CDelgardoMainMenuMsgBoxLayout : public TMsgBoxLayout<CDelgardoMainMenuMsgBox>
	{
	public:
		bool SetLayout();
	};
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008

#ifdef LEM_ADD_LUCKYITEM
	class CLuckyTradeMenuMsgBoxLayout : public TMsgBoxLayout<CLuckyTradeMenuMsgBox>
	{
	public:
		bool SetLayout();
	};
#endif // LEM_ADD_LUCKYITEM
}

#endif // _NEWUICUSOMMESSAGEBOX_H_