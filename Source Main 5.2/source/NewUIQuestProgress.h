//*****************************************************************************
// File: NewUIQuestProgress.h
//*****************************************************************************

#if !defined(AFX_NEWUIQUESTPROGRESS_H__218DE487_9F61_448C_AC69_2AFF5949C15F__INCLUDED_)
#define AFX_NEWUIQUESTPROGRESS_H__218DE487_9F61_448C_AC69_2AFF5949C15F__INCLUDED_

#pragma once

#ifdef ASG_ADD_UI_QUEST_PROGRESS

#include "NewUIBase.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#ifdef ASG_MOD_3D_CHAR_EXCLUSION_UI
#include "NewUIMyQuestInfoWindow.h"
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
#include "QuestMng.h"
#include "UIControls.h"

#define QP_NPC_LINE_MAX		35
#define QP_PLAYER_LINE_MAX	9
#define QP_WORDS_ROW_MAX	64

namespace SEASON3B
{
#ifdef ASG_MOD_3D_CHAR_EXCLUSION_UI
	class CNewUIQuestProgress : public CNewUIObj
#else	// ASG_MOD_3D_CHAR_EXCLUSION_UI
	class CNewUIQuestProgress : public CNewUIObj, public INewUI3DRenderObj
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
	{
	public:
		enum IMAGE_LIST
		{
		// �⺻â
#ifdef ASG_MOD_3D_CHAR_EXCLUSION_UI
			IMAGE_QP_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
			IMAGE_QP_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,		// newui_item_back04.tga	(190,64)
			IMAGE_QP_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
			IMAGE_QP_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
			IMAGE_QP_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)	
			IMAGE_QP_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
			IMAGE_QP_BTN_L = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN ,		// Quest_bt_L.tga	(17,36)
			IMAGE_QP_BTN_R = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 1,		// Quest_bt_R.tga	(17,36)
#else	// ASG_MOD_3D_CHAR_EXCLUSION_UI
			IMAGE_QP_BACK = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN,		// Quest_backimg.jpg
			IMAGE_QP_TOP = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 1,		// Quest_table01.tga	(267,64)
			IMAGE_QP_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,	// newui_item_back02-l.tga	(21,320)
			IMAGE_QP_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
			IMAGE_QP_BOTTOM = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 2,	// Quest_table04.tga	(267,45)
			IMAGE_QP_LINE = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 3,		// Quest_line.tga	(267,21)
			IMAGE_QP_BACK_NPC = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 4,	// Quest_back_npc.tga	(256,128)
			IMAGE_QP_BOX_NPC = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 5,	// Quest_box_npc.tga	(86,96)
			IMAGE_QP_BOX_PLAYER = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 6,// Quest_box_user.tga	(86,96)
			IMAGE_QP_BTN_L = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 7,		// Quest_bt_L.tga	(17,36)
			IMAGE_QP_BTN_R = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 8,		// Quest_bt_R.tga	(17,36)
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
			IMAGE_QP_BTN_COMPLETE = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
			IMAGE_QP_BTN_CLOSE = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
		};

	private:
		enum
		{
#ifdef ASG_MOD_3D_CHAR_EXCLUSION_UI
			QP_WIDTH = 190,
#else	// ASG_MOD_3D_CHAR_EXCLUSION_UI
			QP_WIDTH = 267,
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
			QP_HEIGHT = 429,
		};

		CNewUIManager*		m_pNewUIMng;
#ifndef ASG_MOD_3D_CHAR_EXCLUSION_UI
		CNewUI3DRenderMng*	m_pNewUI3DRenderMng;
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
		POINT				m_Pos;

		CNewUIButton		m_btnProgressL;
		CNewUIButton		m_btnProgressR;
		CNewUIButton		m_btnComplete;
		CNewUIButton		m_btnClose;

		DWORD	m_dwCurQuestIndex;

		char	m_aszNPCWords[QP_NPC_LINE_MAX][QP_WORDS_ROW_MAX];
		int		m_nSelNPCPage;
		int		m_nMaxNPCPage;

		enum LOWER_VIEW_MODE
		{
			NON_PLAYER_WORDS_MODE,
			PLAYER_WORDS_MODE,
			REQUEST_REWARD_MODE
		};
		LOWER_VIEW_MODE	m_eLowerView;

		char	m_aszPlayerWords[QP_PLAYER_LINE_MAX][QP_WORDS_ROW_MAX];
		int		m_anAnswerLine[QM_MAX_ANSWER];
		int		m_nSelAnswer;

		CUIQuestContentsListBox	m_RequestRewardListBox;
		bool	m_bRequestComplete;

		bool	m_bCanClick;

	public:
		CNewUIQuestProgress();
		virtual ~CNewUIQuestProgress();
#ifdef ASG_MOD_3D_CHAR_EXCLUSION_UI
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
#else	// ASG_MOD_3D_CHAR_EXCLUSION_UI
		bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y);
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
#ifndef ASG_MOD_3D_CHAR_EXCLUSION_UI
		void Render3D();
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
		
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

#endif	// ASG_ADD_UI_QUEST_PROGRESS

#endif // !defined(AFX_NEWUIQUESTPROGRESS_H__218DE487_9F61_448C_AC69_2AFF5949C15F__INCLUDED_)
