//*****************************************************************************
// File: NewUINPCDialogue.h
//
// Desc: interface for the CNewUINPCDialogue class.
//		 NPC와 대화 형식으로 진행되는 창 클래스.(2009.09.14)
//		 NPC를 클릭하면 나타나는 UI로 퀘스트 NPC, 겐스 NPC 등이 사용. 
//
// producer: Ahn Sang-Gyu
//*****************************************************************************

#if !defined(AFX_NEWUINPCDIALOGUE_H__5E3DE00E_7D68_42C8_A796_AA368AD96ECC__INCLUDED_)
#define AFX_NEWUINPCDIALOGUE_H__5E3DE00E_7D68_42C8_A796_AA368AD96ECC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef ASG_ADD_UI_NPC_DIALOGUE

#include "NewUIBase.h"
#include "NewUIQuestProgress.h"

#define ND_NPC_LINE_MAX				35	// NPC 대사 최대 줄 수.
#define ND_WORDS_ROW_MAX			64	// 대사 한 줄 당 최대 글자 수 + 종료 문자.
#define ND_QUEST_INDEX_MAX_COUNT	20	// 퀘스트 인덱스 최대 개수.
#define ND_SEL_TEXT_LINE_MAX		((ND_QUEST_INDEX_MAX_COUNT+1)*2)	// 선택문 최대 줄 수.(+1은 "돌아가기" 선택문)
#define ND_SEL_TEXT_PAGE_LIMIT		4	// 선택문 최대 제한 페이지.

namespace SEASON3B
{
	class CNewUINPCDialogue : public CNewUIObj  
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_ND_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
			IMAGE_ND_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,		// newui_item_back01.tga	(190,64)
			IMAGE_ND_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
			IMAGE_ND_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
			IMAGE_ND_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)

			IMAGE_ND_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
			IMAGE_ND_BTN_L = CNewUIQuestProgress::IMAGE_QP_BTN_L,				// Quest_bt_L.tga	(17,36)
			IMAGE_ND_BTN_R = CNewUIQuestProgress::IMAGE_QP_BTN_R,				// Quest_bt_R.tga	(17,36)
			IMAGE_ND_BTN_CLOSE = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,

#ifdef ASG_ADD_GENS_SYSTEM
			IMAGE_ND_CONTRIBUTE_BG = BITMAP_INTERFACE_NPC_DIALOGUE_BEGIN,
#endif	// ASG_ADD_GENS_SYSTEM
		};	
		
	private:
		enum
		{
			ND_WIDTH = 190,
			ND_HEIGHT = 429,
		};
		
		CNewUIManager*		m_pNewUIMng;			// UI 매니저.
		POINT				m_Pos;					// 창의 위치.
		
		CNewUIButton		m_btnProgressL;			// 대화 진행 L 버튼.
		CNewUIButton		m_btnProgressR;			// 대화 진행 R 버튼.
		CNewUIButton		m_btnSelTextL;			// 선택문 페이지 넘기기 L 버튼.
		CNewUIButton		m_btnSelTextR;			// 선택문 페이지 넘기기 R 버튼.
		CNewUIButton		m_btnClose;
		
		char	m_aszNPCWords[ND_NPC_LINE_MAX][ND_WORDS_ROW_MAX];	// NPC 대사 문자열 배열.
		int		m_nSelNPCPage;			// 선택된 NPC 대사 페이지(0 ~ 4).
		int		m_nMaxNPCPage;			// 최대 NPC 대사 페이지수(0 ~ 4).
		
		char	m_aszSelTexts[ND_SEL_TEXT_LINE_MAX][ND_WORDS_ROW_MAX];	// 선택문 문자열 배열.
		int		m_nSelTextCount;		// 선택문 개수.
		int		m_anSelTextLine[ND_QUEST_INDEX_MAX_COUNT+1];	// 각 선택문 줄 수.
		int		m_nSelSelText;			// 선택한 선택문 번호.
		int		m_nSelSelTextPage;		// 선택된 선택문 페이지(0 ~ 3).
		int		m_nMaxSelTextPage;		// 최대 선택문 페이지수(0 ~ 3).
		int		m_anSelTextLinePerPage[ND_SEL_TEXT_PAGE_LIMIT];		// 선택문 각 페이지 당 줄 수.
		int		m_anSelTextCountPerPage[ND_SEL_TEXT_PAGE_LIMIT];	// 선택문 각 페이지 당 개수.

		enum LOWER_VIEW_MODE
		{
			NON_SEL_TEXTS_MODE,			// 선택문 안보여줌.
			SEL_TEXTS_MODE,				// 선택문 보여줌.
		};
		LOWER_VIEW_MODE	m_eLowerView;	// 창의 하단 부분 정보.

		bool	m_bQuestListMode;		// 퀘스트 리스트 모드이면 true.
		DWORD	m_dwCurDlgIndex;		// 현재 진행 중인 대화 인덱스.(m_bQuestListMode가 false일 경우만 사용)
		DWORD	m_adwQuestIndex[ND_QUEST_INDEX_MAX_COUNT];	// 퀘스트 인덱스 배열.(m_bQuestListMode가 true일 경우만 사용)

		bool	m_bCanClick;			// 선택문 및 퀘스트 처리 버튼 클릭 가능 여부.(여러번 클릭 방지)
		DWORD	m_dwContributePoint;	// 기여도.

	public:
		CNewUINPCDialogue();
		virtual ~CNewUINPCDialogue();

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
		
		void SetContents(DWORD dwDlgIndex);
		void SetContributePoint(DWORD dwContributePoint);

		void ProcessQuestListReceive(DWORD* adwSrcQuestIndex, int nIndexCount);
		void ProcessGensJoiningReceive(BYTE byResult, BYTE byInfluence);
		void ProcessGensSecessionReceive(BYTE byResult);
#ifdef PBG_ADD_GENSRANKING
		void ProcessGensRewardReceive(BYTE byResult);
#endif //PBG_ADD_GENSRANKING
		
	private:
		void LoadImages();
		void UnloadImages();
		
		bool UpdateSelTextMouseEvent();
		void RenderBackImage();
		void RenderSelTextBlock();
		void RenderText();
#ifdef ASG_ADD_GENS_SYSTEM
		void RenderContributePoint();
#endif	// ASG_ADD_GENS_SYSTEM
		
		bool ProcessBtns();
		
		void SetCurNPCWords(int nQuestListCount = 0);
		void SetCurSelTexts();
		void SetQuestListText(DWORD* adwSrcQuestIndex, int nIndexCount);
		void CalculateSelTextMaxPage(int nSelTextCount);

		void ProcessSelTextResult();
	};
}

#endif	// ASG_ADD_UI_NPC_DIALOGUE

#endif // !defined(AFX_NEWUINPCDIALOGUE_H__5E3DE00E_7D68_42C8_A796_AA368AD96ECC__INCLUDED_)
