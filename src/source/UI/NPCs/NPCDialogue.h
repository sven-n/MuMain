//*****************************************************************************
// File: NewUINPCDialogue.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/NPCs/NPCDialogueRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "GameLogic/Quests/QuestMng.h"

namespace Rml { class ElementDocument; }

#define ND_NPC_LINE_MAX				35
#define ND_WORDS_ROW_MAX			64
#define ND_QUEST_INDEX_MAX_COUNT	20
#define ND_SEL_TEXT_LINE_MAX		((ND_QUEST_INDEX_MAX_COUNT+1)*2)
#define ND_SEL_TEXT_PAGE_LIMIT		4

// RmlUi-based (npc_dialogue.rml/.rcss) -- no native draws or hybrid item popup left at all, unlike
// CQuestProgress's own m_pSelectedRewardItem boundary: this window has no reward list.
namespace mu::ui::window
{
    class CNPCDialogue : public CObject
    {
    private:
        enum
        {
            ND_WIDTH = 190,
            ND_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        POINT				m_Pos;

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

        RmlModelBinder<NPCDialogueRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

    public:
        CNPCDialogue();
        virtual ~CNPCDialogue();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void Show(bool bShow) override;

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

        void ReloadRmlTheme() override;

        // Invoked directly from RmlUi data-event-click bindings (see BuildRmlUi()), not polled.
        void RmlClickClose();
        void RmlClickNpcPrevPage();
        void RmlClickNpcNextPage();
        void RmlClickAnsPrevPage();
        void RmlClickAnsNextPage();
        void RmlClickSelectAnswer(int nIndex);

    private:
        void BuildRmlUi();
        void SyncRmlModel();

        void SetCurNPCWords(int nQuestListCount = 0);
        void SetCurSelTexts();
        void SetQuestListText(DWORD* adwSrcQuestIndex, int nIndexCount);
        void CalculateSelTextMaxPage(int nSelTextCount);

        void ProcessSelTextResult();
    };
}
