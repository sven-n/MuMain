//*****************************************************************************
// File: NewUIQuestProgressByEtc.h
//*****************************************************************************
#pragma once

#include "NewUIBase.h"
#include "NewUIMyQuestInfoWindow.h"
#include "NewUIQuestProgress.h"
#include "QuestMng.h"

#define QPE_NPC_LINE_MAX	35
#define QPE_PLAYER_LINE_MAX	10
#define QPE_WORDS_ROW_MAX	64

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
            IMAGE_QPE_BTN_L = CNewUIQuestProgress::IMAGE_QP_BTN_L,				// Quest_bt_L.tga	(17,36)
            IMAGE_QPE_BTN_R = CNewUIQuestProgress::IMAGE_QP_BTN_R,				// Quest_bt_R.tga	(17,36)
            IMAGE_QPE_BTN_COMPLETE = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_QPE_BTN_CLOSE = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };

    private:
        enum
        {
            QPE_WIDTH = 190,
            QPE_HEIGHT = 429,
        };

        CNewUIManager* m_pNewUIMng;
        POINT				m_Pos;

        CNewUIButton		m_btnProgressL;
        CNewUIButton		m_btnProgressR;
        CNewUIButton		m_btnComplete;
        CNewUIButton		m_btnClose;

        DWORD	m_dwCurQuestIndex;

        wchar_t	m_aszNPCWords[QPE_NPC_LINE_MAX][QPE_WORDS_ROW_MAX];
        int		m_nSelNPCPage;
        int		m_nMaxNPCPage;

        enum LOWER_VIEW_MODE
        {
            NON_PLAYER_WORDS_MODE,
            PLAYER_WORDS_MODE,
            REQUEST_REWARD_MODE
        };
        LOWER_VIEW_MODE	m_eLowerView;

        wchar_t	m_aszPlayerWords[QPE_PLAYER_LINE_MAX][QPE_WORDS_ROW_MAX];
        int		m_anAnswerLine[QM_MAX_ANSWER];
        int		m_nSelAnswer;

        CUIQuestContentsListBox	m_RequestRewardListBox;
        bool	m_bRequestComplete;
        bool	m_bCanClick;

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
        void EnableCompleteBtn(bool bEnable);

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
