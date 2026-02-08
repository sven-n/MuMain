//*****************************************************************************
// File: NewUINPCDialogue.h
//*****************************************************************************
#pragma once

#include "NewUIBase.h"
#include "NewUIQuestProgress.h"

#define ND_NPC_LINE_MAX				35
#define ND_WORDS_ROW_MAX			64
#define ND_QUEST_INDEX_MAX_COUNT	20
#define ND_SEL_TEXT_LINE_MAX		((ND_QUEST_INDEX_MAX_COUNT+1)*2)
#define ND_SEL_TEXT_PAGE_LIMIT		4

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

        CNewUIManager* m_pNewUIMng;
        POINT				m_Pos;

        CNewUIButton		m_btnProgressL;
        CNewUIButton		m_btnProgressR;
        CNewUIButton		m_btnSelTextL;
        CNewUIButton		m_btnSelTextR;
        CNewUIButton		m_btnClose;

        wchar_t	m_aszNPCWords[ND_NPC_LINE_MAX][ND_WORDS_ROW_MAX];
        int		m_nSelNPCPage;
        int		m_nMaxNPCPage;

        wchar_t	m_aszSelTexts[ND_SEL_TEXT_LINE_MAX][ND_WORDS_ROW_MAX];
        int		m_nSelTextCount;
        int		m_anSelTextLine[ND_QUEST_INDEX_MAX_COUNT + 1];
        int		m_nSelSelText;
        int		m_nSelSelTextPage;
        int		m_nMaxSelTextPage;
        int		m_anSelTextLinePerPage[ND_SEL_TEXT_PAGE_LIMIT];
        int		m_anSelTextCountPerPage[ND_SEL_TEXT_PAGE_LIMIT];

        enum LOWER_VIEW_MODE
        {
            NON_SEL_TEXTS_MODE,
            SEL_TEXTS_MODE,
        };
        LOWER_VIEW_MODE	m_eLowerView;

        bool	m_bQuestListMode;
        DWORD	m_dwCurDlgIndex;
        DWORD	m_adwQuestIndex[ND_QUEST_INDEX_MAX_COUNT];

        bool	m_bCanClick;
        DWORD	m_dwContributePoint;

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
