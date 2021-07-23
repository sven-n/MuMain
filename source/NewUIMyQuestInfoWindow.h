// NewUIMyQuestInfoWindow.h: interface for the CNewUIMyQuestInfoWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMYQUESTINFOWINDOW_H__49097A42_449B_4BA4_88F0_73DC03AA98F9__INCLUDED_)
#define AFX_NEWUIMYQUESTINFOWINDOW_H__49097A42_449B_4BA4_88F0_73DC03AA98F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIGuildInfoWindow.h"
#include "UIControls.h"

#ifdef ASG_MOD_UI_QUEST_INFO
typedef list<DWORD> DWordList;
#endif	// ASG_MOD_UI_QUEST_INFO

namespace SEASON3B
{
	class CNewUIMyQuestInfoWindow  : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			// 기본창
			IMAGE_MYQUEST_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
#ifdef ASG_MOD_UI_QUEST_INFO
			IMAGE_MYQUEST_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,		//. newui_item_back01.tga	(190,64)
#else	// ASG_MOD_UI_QUEST_INFO
			IMAGE_MYQUEST_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,		// newui_item_back04.tga	(190,64)
#endif	// ASG_MOD_UI_QUEST_INFO
			IMAGE_MYQUEST_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
			IMAGE_MYQUEST_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
			IMAGE_MYQUEST_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)
			IMAGE_MYQUEST_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,	//. newui_exit_00.tga

#ifndef ASG_MOD_UI_QUEST_INFO		// 정리시 삭제.
			IMAGE_MYQUEST_TAB_BUTTON = CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON,	//. newui_guild_tab04.tga (56,44,h22)
#endif	// ASG_MOD_UI_QUEST_INFO

			IMAGE_MYQUEST_LINE = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN,
#ifdef ASG_MOD_UI_QUEST_INFO
			IMAGE_MYQUEST_BTN_OPEN = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN+1,
			IMAGE_MYQUEST_BTN_GIVE_UP = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN+2,
			IMAGE_MYQUEST_TAB_BACK = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN+3,
			IMAGE_MYQUEST_TAB_SMALL = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN+4,
			IMAGE_MYQUEST_TAB_BIG = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN+5,
#endif	// ASG_MOD_UI_QUEST_INFO
		};
		enum
		{
			MYQUESTINFO_WINDOW_WIDTH = 190,
			MYQUESTINFO_WINDOW_HEIGHT = 429,
		};

#ifdef ASG_MOD_UI_QUEST_INFO
		enum TAB_BUTTON_INDEX
		{
			TAB_NON	= -1,
			TAB_QUEST = 0,		// 퀘스트
			TAB_JOB_CHANGE,		// 전직 퀘스트
			TAB_CASTLE_TEMPLE,	// 캐슬, 환영사원
		};
#else	// ASG_MOD_UI_QUEST_INFO
		enum TAB_BUTTON_INDEX
		{
			TAB_QUEST = 0,		// 퀘스트
			TAB_CASTLE,		// 캐슬
			TAB_TEMPLE,		// 환영사원
		};
#endif	// ASG_MOD_UI_QUEST_INFO
		
	public:
		CNewUIMyQuestInfoWindow();
		virtual ~CNewUIMyQuestInfoWindow();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();	//. 3.3f
		
		void OpenningProcess();
		void ClosingProcess();

#ifdef ASG_MOD_UI_QUEST_INFO
		void UnselectQuestList();
		void SetCurQuestList(DWordList* pDWordList);
		void SetSelQuestSummary();
		void SetSelQuestRequestReward();

		void QuestOpenBtnEnable(bool bEnable);
		void QuestGiveUpBtnEnable(bool bEnable);

		DWORD GetSelQuestIndex();
#endif	// ASG_MOD_UI_QUEST_INFO

	private:
		void LoadImages();
		void UnloadImages();
		
		void SetButtonInfo();
		
		bool BtnProcess();

		void RenderFrame();
		void RenderSubjectTexts();
#ifdef ASG_MOD_UI_QUEST_INFO
		void RenderQuestInfo();
		void RenderJobChangeContents();
		void RenderJobChangeState();
		void RenderCastleInfo();
		void RenderTempleInfo();
#else	// ASG_MOD_UI_QUEST_INFO
		void RenderQuestTexts();
		void RenderTabQuest();
		void RenderTabCastle();
		void RenderTabTemple();
		void RenderButtons();
#endif	// ASG_MOD_UI_QUEST_INFO

#ifdef ASG_MOD_UI_QUEST_INFO
		TAB_BUTTON_INDEX UpdateTabBtn();
		void RenderTabBtn();

		void SetMessage(int nGlobalTextIndex);
#endif	// ASG_MOD_UI_QUEST_INFO

	private:
		CNewUIManager* m_pNewUIMng;
		POINT m_Pos;

#ifndef ASG_MOD_UI_QUEST_INFO		// 정리시 삭제.
		CNewUIRadioGroupButton m_BtnTab;
#endif	// ASG_MOD_UI_QUEST_INFO
		CNewUIButton m_BtnExit;
#ifdef ASG_MOD_UI_QUEST_INFO
		CNewUIButton	m_btnQuestOpen;		// 퀘스트 열기 버튼.
		CNewUIButton	m_btnQuestGiveUp;	// 퀘스트 포기 버튼.

		TAB_BUTTON_INDEX m_eTabBtnIndex;	// 탭버튼 인덱스.

		CUICurQuestListBox		m_CurQuestListBox;		// 진행중인 퀘스트 리스트 박스.
		CUIQuestContentsListBox	m_QuestContentsListBox;	// 퀘스트 내용 리스트 박스.
		char			m_aszMsg[2][64];	// 2줄 메시지 용.
		int				m_nMsgLine;			// 메시지 줄 수.
#endif	// ASG_MOD_UI_QUEST_INFO
	};
}

#endif // !defined(AFX_NEWUIMYQUESTINFOWINDOW_H__49097A42_449B_4BA4_88F0_73DC03AA98F9__INCLUDED_)
