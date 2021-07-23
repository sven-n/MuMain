// NewUIKanturu2ndEnterNpc.h: interface for the CNewUIKanturu2ndEnterNpc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIKANTURU2NDENTERNPC_H__4CDE30B6_3570_47BA_9401_0EA282BA1949__INCLUDED_)
#define AFX_NEWUIKANTURU2NDENTERNPC_H__4CDE30B6_3570_47BA_9401_0EA282BA1949__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIButton.h"

namespace SEASON3B
{
	
	class CNewUIKanturu2ndEnterNpc  : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			// 기본창
			IMAGE_KANTURU2ND_TOP = CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP,	
			IMAGE_KANTURU2ND_MIDDLE = CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE,
			IMAGE_KANTURU2ND_BOTTOM = CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM,
			IMAGE_KANTURU2ND_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	
			IMAGE_KANTURU2ND_BTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
		};
		enum
		{
			KANTURU2ND_ENTER_WINDOW_WIDTH = 230,
			KANTURU2ND_ENTER_WINDOW_HEIGHT = 267,
		};

		enum MSGBOX_TYPE
		{
			POPUP_NONE = 0,
			POPUP_USER_OVER,	// 입장 가능한 인원 수 초과
			POPUP_NOT_MUNSTONE,	// 문스톤 펜던트 착용하고 있는 않다.
			POPUP_FAILED,		// 입장 가능한 상태가 아님
			POPUP_FAILED2,		// 이상한 오류로 실패
			POPUP_UNIRIA = 5,	// 유니리아를 탑승하고 이동 불과
			POPUP_CHANGERING,	// 변신반지 착용하면 이동 불과
			POPUP_NOT_HELPER,	// 날개, 망토, 디노란트, 펜릴, 다크호스를 가지고 있지 않을 경우
		};

	public:
		CNewUIKanturu2ndEnterNpc();
		virtual ~CNewUIKanturu2ndEnterNpc();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();	//. 10.1f

		void SetNpcObject(OBJECT* pObj);
		bool IsNpcAnimation();
		void SetNpcAnimation(bool bValue);
		bool IsEnterRequest();
		void SetEnterRequest(bool bValue);
		void CreateMessageBox(BYTE btResult);

		// 패킷 관련 함수
		void ReceiveKanturu3rdInfo(BYTE btState, BYTE btDetailState, BYTE btEnter, BYTE btUserCount, int iRemainTime);
		void ReceiveKanturu3rdEnter(BYTE btResult);
		void SendRequestKanturu3rdInfo();
		void SendRequestKanturu3rdEnter();

	private:
		void Initialize();

		void LoadImages();
		void UnloadImages();

		void SetButtonInfo();

		bool BtnProcess();

		void RenderFrame();
		void RenderButtons();
		void RenderTexts();

	private:
		CNewUIManager* m_pNewUIMng;
		POINT m_Pos;

		BYTE m_byState;	//	칸투르 3차 상태 

		bool m_bNpcAnimation;	//	NPC 에니메이션 
		OBJECT*	m_pNpcObject;		//	NPC 오브젝트
		
		bool m_bEnterRequest;	// 입장요청

		// 시간 관련 변수
		DWORD m_dwRefreshTime;	// 정보갱신 시간 재는 변수(4초마다 갱신)
		DWORD m_dwRefreshButtonGapTime;	// 정보갱신 버튼 1초 

		// 텍스트 관련 변수
		unicode::t_char m_strSubject[MAX_GLOBAL_TEXT_STRING];		// 제목
		unicode::t_char m_strStateText[KANTURU2ND_STATETEXT_MAX][MAX_GLOBAL_TEXT_STRING];	// 상태별 텍스트
		int	m_iStateTextNum;							// 상태별 텍스트 갯수

		// 버튼들
		CNewUIButton m_BtnEnter;
		CNewUIButton m_BtnRefresh;
		CNewUIButton m_BtnClose;
	};
	

	class CNewUIKanturuInfoWindow  : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			// 기본창
			IMAGE_KANTURUINFO_WINDOW = BITMAP_KANTURU_INFO_BEGIN,	
		};
		enum
		{
			KANTURUINFO_WINDOW_WIDTH = 99,
			KANTURUINFO_WINDOW_HEIGHT = 78,
		};
	public:
		CNewUIKanturuInfoWindow();
		virtual ~CNewUIKanturuInfoWindow();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();	//. 1.92f
		float GetKeyEventOrder();	//. 9.1f

		void SetTime(int iTimeLimit);

	private:
		void LoadImages();
		void UnloadImages();

		void RenderFrame();
		void RenderInfo();

	private:
		CNewUIManager* m_pNewUIMng;
		POINT m_Pos;
		
		int m_iMinute;
		int m_iSecond;
		DWORD m_dwSyncTime;
	};
	
}

#endif // !defined(AFX_NEWUIKANTURU2NDENTERNPC_H__4CDE30B6_3570_47BA_9401_0EA282BA1949__INCLUDED_)
