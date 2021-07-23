// NewUICommonMessageBox.h: interface for the NewUICommonMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_)
#define AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIMessageBox.h"
#include "NewUI3DRenderMng.h"

namespace SEASON3B
{
	// Common 메세지박스 타입
	enum
	{
		MSGBOX_COMMON_TYPE_OK,			// OK 버튼 메세지박스
		MSGBOX_COMMON_TYPE_OKCANCEL,	// OK, CANCEL 버튼 메세지박스
	};
	
	// 메세지박스 안에서 폰트 타입
	enum
	{
		MSGBOX_FONT_NORMAL,
		MSGBOX_FONT_BOLD,
	};

	// 상수값들
	enum
	{
		SCREEN_WIDTH = 640,
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		SCREEN_HEIGHT = 480,
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

		MSGBOX_WIDTH = 230,
		MSGBOX_TOP_HEIGHT = 67,
		MSGBOX_BOTTOM_HEIGHT = 50,
		MSGBOX_MIDDLE_HEIGHT = 15,

		MSGBOX_BACK_BLANK_WIDTH = 8,
		MSGBOX_BACK_BLANK_HEIGHT = 10,

		MSGBOX_TEXT_TOP_BLANK = 35,
		MSGBOX_TEXT_MAXWIDTH = 180,

		MSGBOX_LINE_WIDTH = 223,	
		MSGBOX_LINE_HEIGHT = 21,
		
		MSGBOX_SEPARATE_LINE_WIDTH = 205,
		MSGBOX_SEPARATE_LINE_HEIGHT = 2,

		MSGBOX_BTN_WIDTH = 54,
		MSGBOX_BTN_HEIGHT = 30,
		MSGBOX_BTN_BOTTOM_BLANK = 20,

		MSGBOX_BTN_EMPTY_SMALL_WIDTH = 64,
		MSGBOX_BTN_EMPTY_WIDTH = 108,
		MSGBOX_BTN_EMPTY_BIG_WIDTH = 180,
		MSGBOX_BTN_EMPTY_HEIGHT = 29,
	};

	// 메세비 박스 안에서 문자 데이타
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

	class CNewUIMessageBoxButton
	{
	public:
		enum EVENT_STATE
		{
			EVENT_NONE = 0,
			EVENT_BTN_HOVER,
			EVENT_BTN_DOWN,
		};

		enum BTN_SIZE_TYPE
		{
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
			// 위의 4경우를 제외하고 사이즈 지정해서 사용할경우(기타)
			MSGBOX_BTN_CUSTOM = 0,
#endif //KJH_ADD_INGAMESHOP_UI_SYSTEM
			MSGBOX_BTN_SIZE_OK,
			MSGBOX_BTN_SIZE_EMPTY,
			MSGBOX_BTN_SIZE_EMPTY_SMALL,
			MSGBOX_BTN_SIZE_EMPTY_BIG,
		};

		CNewUIMessageBoxButton();
		~CNewUIMessageBoxButton();

		bool IsMouseIn();
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		void SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType = MSGBOX_BTN_CUSTOM, bool bClickEffect = false);
		void MoveTextPos(int iX, int iY);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
		void SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType = MSGBOX_BTN_SIZE_OK);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
		void SetText(const unicode::t_char* strText);
		void AddBlank(int iAddLine);
		
		void SetEnable(bool bEnable) { m_bEnable = bEnable; }

		void SetPos(float x, float y) { m_x = x; m_y = y; }
		float GetPosX() { return m_x; }
		float GetPosY() { return m_y; }
		float GetWidth() { return m_width; }
		float GetHeight() { return m_height; }

		void ClearEventState() { m_EventState = EVENT_NONE; } 
		EVENT_STATE GetEventState() { return m_EventState; }
		
		virtual void Update();
		virtual void Render();

	private:
		bool m_bEnable;

		DWORD m_dwTexType;
		DWORD m_dwSizeType;
		
		unicode::t_string m_strText;
		float m_x, m_y, m_width, m_height;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		float m_fButtonWidth;
		float m_fButtonHeight;
		int		m_iMoveTextPosX;
		int		m_iMoveTextPosY;
		bool	m_bClickEffect;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

		EVENT_STATE m_EventState;	// 마우스 상태
	};

	class CNewUICommonMessageBox : public CNewUIMessageBoxBase
	{
	public:
		CNewUICommonMessageBox();
		~CNewUICommonMessageBox();

		DWORD GetType();

		bool Create(DWORD dwType, float fPriority = 3.f);
		bool Create(DWORD dwType, const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL, float fPriority = 3.f);
		
		void SetPos(int x, int y);

		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT Close(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	
		bool Update();
		bool Render();

		void LockOkButton();
#ifdef PBG_ADD_NAMETOPMSGBOX
	protected:
#else //PBG_ADD_NAMETOPMSGBOX
	private:
#endif //PBG_ADD_NAMETOPMSGBOX
		void SetAddCallbackFunc();
#ifdef PBG_ADD_NAMETOPMSGBOX
		int SeparateText(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL, int _TextSize = MSGBOX_TEXT_MAXWIDTH);
#else //PBG_ADD_NAMETOPMSGBOX
		int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);
#endif //PBG_ADD_NAMETOPMSGBOX
		void SetButtonInfo();
		void AddButtonBlank(int iAddLine);
#ifdef PBG_ADD_NAMETOPMSGBOX
		void AddButtonBlank(int iAddLine, int _iImgSize);
#endif //PBG_ADD_NAMETOPMSGBOX
		
		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		DWORD m_dwType;	// 메세지 타입
		type_vector_msgdata m_MsgDataList;

		// button
		CNewUIMessageBoxButton m_BtnOk;
		CNewUIMessageBoxButton m_BtnCancel;
	};

	//////////////////////////////////////////////////////////////////////////

	class CNewUI3DItemCommonMsgBox : public CNewUIMessageBoxBase, public INewUI3DRenderObj
	{
		enum
		{
			MSGBOX_TEXT_MAXWIDTH_3DITEM = 120,
			MSGBOX_TEXT_LEFT_BLANK_3DITEM = 60,
			MSGBOX_3DITEM_WIDTH = 40,
			MSGBOX_3DITEM_HEIGHT = 40,	
		};
	public:
		CNewUI3DItemCommonMsgBox();
		~CNewUI3DItemCommonMsgBox();

		DWORD GetType();

		bool Create(DWORD dwType, float fPriority = 3.f);
		bool Create(DWORD dwType, const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL, float fPriority = 3.f);
		void Release();
		
		void AddMsg(const type_string& strMsg, DWORD dwColor = CLRDW_WHITE, BYTE byFontType = MSGBOX_FONT_NORMAL);
		
		void Set3DItem(ITEM* pItem);
		void SetItemValue(int iValue);
		int GetItemValue();

		static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT Close(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	
		bool Update();
		bool Render();
		void Render3D();

		bool IsVisible() const;

	private:
		void SetAddCallbackFunc();
		int SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType);

		void SetButtonInfo();
		void AddButtonBlank(int iAddLine);

		void RenderFrame();
		void RenderTexts();
		void RenderButtons();

		DWORD m_dwType;	// 메세지 타입
		ITEM m_Item;	
		int m_iItemValue;	// 아이템 입력 가격

		type_vector_msgdata m_MsgDataList;

		// button
		CNewUIMessageBoxButton m_BtnOk;
		CNewUIMessageBoxButton m_BtnCancel;
	};

	class CFenrirRepairMsgBox : public CNewUICommonMessageBox
	{
	public:
		void SetSourceIndex(int iIndex);
		void SetTargetIndex(int iIndex);
		int GetSourceIndex();
		int GetTargetIndex();

	private:
		int m_iSourceIndex;
		int m_iTargetIndex;
	};

#ifdef PBG_MOD_SECRETITEM
	class CBuffUseMsgBox : public CNewUICommonMessageBox
	{
	public:
		void SetInvenIndex(int _nIndex);
		int GetInvenIndex();

	private:
		int m_nIndex;
	};
#endif //PBG_MOD_SECRETITEM
	//////////////////////////////////////////////////////////////////////////
	///////////////// Common Message Box Layout //////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	// 서버와의 접속이 종료 메세지박스
	class CServerLostMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	
	// 길드 가입 요청 메세지박스
	class CGuildRequestMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 길드에서 방출하는 메세지박스
	class CGuildFireMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	
	// 3차 체인지업 웨어울프 쿼렐 NPC 입장 메세지박스
	class CMapEnterWerwolfMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 3차 체인지업 문지기 NPC 입장 메세지박스
	class CMapEnterGateKeeperMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	
	// 파티 신청 메세지박스
	class CPartyMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 거래 신청 메세지박스
	class CTradeMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 거래 경고 메세지박스
	class CTradeAlertMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	
	// 길드전 신청 메세지박스
	class CGuildWarMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 배틀축구전 신청 메세지박스
	class CBattleSoccerMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 서버 이전할때 주민등록번호 틀렸을 경우 메세지박스
	class CServerImmigrationErrorMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 개인상점 만들겠냐는 메세지박스
	class CPersonalshopCreateMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	
	// 펜릴 수리 메세지박스
	class CFenrirRepairMsgBoxLayout : public TMsgBoxLayout<CFenrirRepairMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 인피니티 애로우 스킬 취소 메세지박스
	class CInfinityArrowCancelMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER		// 마력증대
	// 마력증대 버프 스킬 취소 메세지 박스
	class CBuffSwellOfMPCancelMsgBoxLayOut : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER

	// 보석 통합 확인 메세지박스
	class CGemIntegrationUnityCheckMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 보석 통합 서버결과 메세지박스
	class CGemIntegrationUnityResultMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 보석 해체 확인 메세지박스
	class CGemIntegrationDisjointCheckMsgBoxLayout :public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 보석 해체 서버결과 메세지박스
	class CGemIntegrationDisjointResultMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);	
	};
	
	// 카오스캐슬 입장 체크 메세지박스
	class CChaosCastleTimeCheckMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// PC방 이벤트 아이템 지급 메세지박스
	class CPCRoomItemGiveLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	
	// 도우미앨런 추석 이벤트 아이템 지급 메세지박스
	class CHarvestEventLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 화이트 앤젤 이벤트 메세지박스
	class CWhiteAngelEventLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 사용할 수 없는 단어가 포함되었다는 메세지박스
	class CCanNotUseWordMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:	
		bool SetLayout();
	};

	// 조합하시겠습니까 물어보는 메세지박스
	class CMixCheckMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
	// 부활 아이템 사용 여부 물어보는 메세지박스
	class CUseReviveCharmMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
	// 이동 아이템 사용 여부 물어보는 메세지박스
	class CUsePortalCharmMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	// 이동 아이템 복귀 여부 물어보는 메세지박스
	class CReturnPortalCharmMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef YDG_ADD_NEW_DUEL_UI
	// 결투장 만원 메세지박스
	class CDuelCreateErrorMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	// 결투장 관전 실패 메세지박스
	class CDuelWatchErrorMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif	// YDG_ADD_NEW_DUEL_UI

#ifdef YDG_ADD_DOPPELGANGER_UI
	// 도플갱어 입장 안내 메세지박스
	class CDoppelGangerMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif	// YDG_ADD_DOPPELGANGER_UI
	
	class CGuildRelationShipMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCastleMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CSiegeLevelMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	class CSiegeGiveUpMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	class CGatemanMoneyMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	class CGatemanFailMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
	
#ifdef ASG_MOD_UI_QUEST_INFO
	// 퀘스트 포기할 것인가? 물어보는 메시지 박스.
	class CQuestGiveUpMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif	// ASG_MOD_UI_QUEST_INFO

#ifdef ASG_ADD_TIME_LIMIT_QUEST
	// 시간제 퀘스트 수행 개수 제한 메시지 박스.
	class CQuestCountLimitMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif	// ASG_ADD_TIME_LIMIT_QUEST

	//////////////////// 3DItem Common 메세지박스 Layout //////////////////////////////

	// 값 비싼 고가 물품 확인 메세지박스
	class CHighValueItemCheckMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 열매 사용 메세지박스
	class CUseFruitMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

#ifdef PSW_FRUIT_ITEM
	// 열매 사용 메세지박스 
	class CUsePartChargeFruitMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif //PSW_FRUIT_ITEM

	// 제련 설명 메세지박스
	class COsbourneMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};


	// 개인상점 판매할 가격 확인 메세지박스
	class CPersonalShopItemValueCheckMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	// 개인상점에서 아이템 구매 확인 메세지박스
	class CPersonalShopItemBuyMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

#ifndef KJH_DEL_PC_ROOM_SYSTEM		// #ifndef
	// PC방 포인트로 아이템 구매 확인 메세지박스
	class CPCRoomPointItemBuyMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif // KJH_DEL_PC_ROOM_SYSTEM

	// 길드해체관련
	class CGuildOutPerson : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CGuildBreakMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CGuildPerson_Get_Out : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CGuildPerson_Cancel_Position_MsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CUnionGuild_Break_MsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CUnionGuild_Out_MsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CMaster_Level_Interface : public TMsgBoxLayout<CNewUICommonMessageBox>//크라이울프 계약할거냐고 물어봄.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCry_Wolf_Get_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>//크라이울프 계약할거냐고 물어봄.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCry_Wolf_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>//크라이울프 계약성립.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCry_Wolf_Set_Temple1 : public TMsgBoxLayout<CNewUICommonMessageBox>//계약을 해줄것을 호소.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCry_Wolf_Dont_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>//탈거타고 계약하지말오.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCry_Wolf_Dont_Set_Temple1 : public TMsgBoxLayout<CNewUICommonMessageBox>//탈거타고 계약하지말오.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCry_Wolf_Wat_Set_Temple1 : public TMsgBoxLayout<CNewUICommonMessageBox>//탈거타고 계약하지말오.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCry_Wolf_Destroy_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>//탈거타고 계약하지말오.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCry_Wolf_Ing_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>//탈거타고 계약하지말오.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	class CCry_Wolf_Result_Set_Temple : public TMsgBoxLayout<CNewUICommonMessageBox>//탈거타고 계약하지말오.
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#ifdef PBG_ADD_SANTAINVITATION
	//산타마을초대장 메세지 박스
	class CUseSantaInvitationMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif //PBG_ADD_SANTAINVITATION

#ifdef LDK_ADD_SNOWMAN_NPC
	class CSantaTownLeaveMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif //LDK_ADD_SNOWMAN_NPC
	
#ifdef LDK_ADD_SANTA_NPC
	class CSantaTownSantaMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:	
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif //LDK_ADD_SANTA_NPC
	
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	//행운의 동전 등록 부족메시지
	class CUseRegistLuckyCoinMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

#ifdef PBG_MOD_LUCKYCOINEVENT
	//------------------------------------------
	// 행운의동전 등록시 초과된경우(09.07.15 이벤트 기획 변경)
	class CRegistOverLuckyCoinMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif //PBG_MOD_LUCKYCOINEVENT

	//------------------------------------------
	// 행운의동전 교환시 동전부족 메세지
	class CExchangeLuckyCoinMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};

	//------------------------------------------	
	//행운의 동전 교환시 인벤공간부족 메시지
	class CExchangeLuckyCoinInvenErrMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008

#ifdef LDK_ADD_GAMBLE_SYSTEM
	class CGambleBuyMsgBoxLayout : public TMsgBoxLayout<CNewUI3DItemCommonMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);		
	};
#endif //LDK_ADD_GAMBLE_SYSTEM

#ifdef LDK_ADD_EMPIREGUARDIAN_UI
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//제국 수호군 메시지 박스
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class CEmpireGuardianMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif //LDK_ADD_EMPIREGUARDIAN_UI

#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//통합시장 메시지 박스
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class CUnitedMarketPlaceMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
#ifdef PBG_MOD_SECRETITEM
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//버프 적용시 메시지 박스
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class CBuffUseOverlapMsgBoxLayout : public TMsgBoxLayout<CBuffUseMsgBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif //PBG_MOD_SECRETITEM
#ifdef LEM_ADD_LUCKYITEM
	class CLuckyItemMsgBoxLayout : public TMsgBoxLayout<CNewUICommonMessageBox>
	{
	public:
		bool SetLayout();
		static CALLBACK_RESULT OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
		static CALLBACK_RESULT CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	};
#endif // LEM_ADD_LUCKYITEM
}

#endif // !defined(AFX_NEWUICOMMONMESSAGEBOX_H__AA370602_D171_41DC_9A79_345D75F678D4__INCLUDED_)
