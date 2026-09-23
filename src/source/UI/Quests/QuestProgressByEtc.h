//*****************************************************************************
// File: NewUIQuestProgressByEtc.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Quests/QuestProgressRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "GameLogic/Quests/QuestMng.h"

namespace Rml { class ElementDocument; }

#define QPE_NPC_LINE_MAX	35
#define QPE_PLAYER_LINE_MAX	10
#define QPE_WORDS_ROW_MAX	64

// RmlUi-based (quest_progress_etc.rml, shares quest_progress.rcss with CQuestProgress's own
// quest_progress.rml -- see that file's own header comment for why two .rml files, one .rcss).
// Unlike CQuestProgress, this window never shows an NPC-name/player-name preamble -- those
// elements simply don't exist in this class's own .rml, a structural difference, not a data toggle
// (see QuestProgressRmlModel.h's own comment on npcName/playerName).
namespace mu::ui::window
{
    class CQuestProgressByEtc : public CObject
    {
    public:
        enum LOWER_VIEW_MODE
        {
            NON_PLAYER_WORDS_MODE,
            PLAYER_WORDS_MODE,
            REQUEST_REWARD_MODE
        };

        enum
        {
            QPE_WIDTH = 190,
            QPE_HEIGHT = 429,
        };

    public:
        CQuestProgressByEtc();
        virtual ~CQuestProgressByEtc();
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
        void SetContents(DWORD dwQuestIndex);
        void EnableCompleteBtn(bool bEnable);

        void ReloadRmlTheme();

        // Invoked directly from RmlUi data-event-click bindings (see BuildRmlUi()), not polled.
        void RmlClickClose();
        void RmlClickPrevPage();
        void RmlClickNextPage();
        void RmlClickSelectAnswer(int nAnswerIndex);
        void RmlClickSelectReward(int nRewardIndex);
        void RmlClickComplete();

    private:
        void BuildRmlUi();
        void SyncRmlModel();

        void SetCurNPCWords();
        void SetCurRequestReward();

    private:
        CManager* m_pNewUIMng;
        POINT m_Pos;

        DWORD m_dwCurQuestIndex = 0;

        wchar_t m_aszNPCWords[QPE_NPC_LINE_MAX][QPE_WORDS_ROW_MAX] = {};
        int m_nSelNPCPage = 0;
        int m_nMaxNPCPage = 0;

        LOWER_VIEW_MODE m_eLowerView = NON_PLAYER_WORDS_MODE;

        bool m_bRequestComplete = false;
        bool m_bCanClick = false;

        // Raw reward-row data for click handling, alongside the model's display-only Entry list --
        // same split CMyQuestInfoWindow's own m_ContentRows/RewardModel::Entry uses.
        std::vector<UI::Quests::RewardModel::RowData> m_RewardRows;

        // Selected reward-item's info popup -- still native-rendered every frame (::RenderItemInfo())
        // rather than ported to RmlUi. See CMyQuestInfoWindow's own m_pSelectedRewardItem.
        ITEM* m_pSelectedRewardItem = nullptr;

        RmlModelBinder<QuestProgressRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
    };
}
