//*****************************************************************************
// File: NewUIQuestProgressByEtc.h
//
// Desc: interface for the CNewUIQuestProgressByEtc class.
//		 신규 (기타 상황으로 인한)퀘스트 진행 창 클래스.(2009.06.24)
//		 NPC를 클릭하여 진행되는 퀘스트가 아닌 퀘스트 진행은 이 UI로 진행.
//
// producer: Ahn Sang-Gyu
//*****************************************************************************

#if !defined(AFX_NEWUIQUESTPROGRESSBYETC_H__0D2110D4_FB24_4DA3_A2BC_198111DF4413__INCLUDED_)
#define AFX_NEWUIQUESTPROGRESSBYETC_H__0D2110D4_FB24_4DA3_A2BC_198111DF4413__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC

#include "NewUIBase.h"
#include "NewUIMyQuestInfoWindow.h"
#include "NewUIQuestProgress.h"
#include "QuestMng.h"

#define QPE_NPC_LINE_MAX	35	// NPC 대사 최대 줄 수.
#define QPE_PLAYER_LINE_MAX	10	// 플래이어 대사 최대 줄 수.
#define QPE_WORDS_ROW_MAX	64	// 대사 한 줄 당 최대 글자 수 + 종료 문자.

namespace SEASON3B
{
	class CNewUIQuestProgressByEtc : public CNewUIObj  
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_QPE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
			IMAGE_QPE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,		// newui_item_back04.tga	(190,64)
			IMAGE_QPE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
			IMAGE_QPE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
			IMAGE_QPE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)

			IMAGE_QPE_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
#ifdef ASG_ADD_UI_QUEST_PROGRESS
			IMAGE_QPE_BTN_L = CNewUIQuestProgress::IMAGE_QP_BTN_L,				// Quest_bt_L.tga	(17,36)
			IMAGE_QPE_BTN_R = CNewUIQuestProgress::IMAGE_QP_BTN_R,				// Quest_bt_R.tga	(17,36)
#endif	// ASG_ADD_UI_QUEST_PROGRESS
			IMAGE_QPE_BTN_COMPLETE = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
			IMAGE_QPE_BTN_CLOSE = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
		};	

	private:
		enum
		{
			QPE_WIDTH = 190,
			QPE_HEIGHT = 429,
		};

		CNewUIManager*		m_pNewUIMng;			// UI 매니저.
		POINT				m_Pos;					// 창의 위치.
		
		CNewUIButton		m_btnProgressL;			// 대화 진행 L 버튼.
		CNewUIButton		m_btnProgressR;			// 대화 진행 R 버튼.
		CNewUIButton		m_btnComplete;			// 퀘스트 완료 버튼.
		CNewUIButton		m_btnClose;

		DWORD	m_dwCurQuestIndex;		// 현재 진행 중인 퀘스트 인덱스.

		char	m_aszNPCWords[QPE_NPC_LINE_MAX][QPE_WORDS_ROW_MAX];	// NPC 대사.
		int		m_nSelNPCPage;			// 선택된 NPC 대사 페이지(0 ~ 4).
		int		m_nMaxNPCPage;			// 최대 NPC 대사 페이지수(0 ~ 4).

		enum LOWER_VIEW_MODE
		{
			NON_PLAYER_WORDS_MODE,		// 플래이어 대사 안보여줌.
			PLAYER_WORDS_MODE,			// 플래이어 대사 보여줌.
			REQUEST_REWARD_MODE			// 요구 사항, 보상 보여줌.
		};
		LOWER_VIEW_MODE	m_eLowerView;	// 창의 하단 부분 정보.

		char	m_aszPlayerWords[QPE_PLAYER_LINE_MAX][QPE_WORDS_ROW_MAX];	// 플래이어 대사.
		int		m_anAnswerLine[QM_MAX_ANSWER];	// 각 선택문 줄 수.
		int		m_nSelAnswer;			// 선택한 선택문 번호.

		CUIQuestContentsListBox	m_RequestRewardListBox;	// 요구 사항, 보상 리스트 박스.
		bool	m_bRequestComplete;		// 요구 사항 만족 여부.
		
		bool	m_bCanClick;			// 선택문 및 퀘스트 처리 버튼 클릭 가능 여부.(여러번 클릭 방지)

	public:
		CNewUIQuestProgressByEtc();
		virtual ~CNewUIQuestProgressByEtc();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		bool IsVisible() const;
		
		float GetLayerDepth();	//. 3.1f
		
		void ProcessOpening();
		bool ProcessClosing();
		
		void SetContents(DWORD dwQuestIndex);
#ifdef ASG_MOD_QUEST_OK_BTN_DISABLE
		void EnableCompleteBtn(bool bEnable);
#endif	// ASG_MOD_QUEST_OK_BTN_DISABLE
		
	private:
		void LoadImages();
		void UnloadImages();
		
		bool UpdateSelTextMouseEvent();
		void RenderBackImage();
		void RenderSelTextBlock();
		void RenderText();
		
		bool ProcessBtns();
		
		void SetCurNPCWords();
		void SetCurPlayerWords();
		void SetCurRequestReward();
	};
}

#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC

#endif // !defined(AFX_NEWUIQUESTPROGRESSBYETC_H__0D2110D4_FB24_4DA3_A2BC_198111DF4413__INCLUDED_)
