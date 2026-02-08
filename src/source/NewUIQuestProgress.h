//*****************************************************************************
// File: NewUIQuestProgress.h
//*****************************************************************************
#pragma once

#include "NewUIBase.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIMyQuestInfoWindow.h"
#include "QuestMng.h"
#include "UIControls.h"

#define QP_NPC_LINE_MAX		35
#define QP_PLAYER_LINE_MAX	9
#define QP_WORDS_ROW_MAX	64

namespace SEASON3B
{
    class CNewUIQuestProgress : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            // �⺻â
            IMAGE_QP_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
            IMAGE_QP_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,		// newui_item_back04.tga	(190,64)
            IMAGE_QP_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
            IMAGE_QP_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
            IMAGE_QP_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)
            IMAGE_QP_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_QP_BTN_L = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN,		// Quest_bt_L.tga	(17,36)
            IMAGE_QP_BTN_R = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 1,		// Quest_bt_R.tga	(17,36)
            IMAGE_QP_BTN_COMPLETE = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_QP_BTN_CLOSE = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };

    private:
        enum
        {
            QP_WIDTH = 190,
            QP_HEIGHT = 429,
        };

        CNewUIManager* m_pNewUIMng;
        POINT				m_Pos;

        CNewUIButton		m_btnProgressL;
        CNewUIButton		m_btnProgressR;
        CNewUIButton		m_btnComplete;
        CNewUIButton		m_btnClose;

        DWORD	m_dwCurQuestIndex;

        wchar_t	m_aszNPCWords[QP_NPC_LINE_MAX][QP_WORDS_ROW_MAX];
        int		m_nSelNPCPage;
        int		m_nMaxNPCPage;

        enum LOWER_VIEW_MODE
        {
            NON_PLAYER_WORDS_MODE,
            PLAYER_WORDS_MODE,
            REQUEST_REWARD_MODE
        };
        LOWER_VIEW_MODE	m_eLowerView;

        wchar_t	m_aszPlayerWords[QP_PLAYER_LINE_MAX][QP_WORDS_ROW_MAX];
        int		m_anAnswerLine[QM_MAX_ANSWER];
        int		m_nSelAnswer;

        CUIQuestContentsListBox	m_RequestRewardListBox;
        bool	m_bRequestComplete;

        bool	m_bCanClick;

    public:
        CNewUIQuestProgress();
        virtual ~CNewUIQuestProgress();
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
