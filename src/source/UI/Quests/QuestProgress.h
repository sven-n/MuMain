//*****************************************************************************
// File: NewUIQuestProgress.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Quests/QuestProgressRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "GameLogic/Quests/QuestMng.h"
#include "Core/Globals/_TextureIndex.h" // BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN

namespace Rml { class ElementDocument; }

#define QP_NPC_LINE_MAX		35
#define QP_PLAYER_LINE_MAX	9
#define QP_WORDS_ROW_MAX	64

// RmlUi-based (quest_progress.rml/.rcss, shared with CQuestProgressByEtc's own
// quest_progress_etc.rml, see quest_progress.rcss's own header comment), except the selected
// reward item's info popup, which stays a native per-frame draw -- same permanent hybrid boundary
// CMyQuestInfoWindow's own m_pSelectedRewardItem already established.
namespace mu::ui::window
{
    class CQuestProgress : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            // Kept only because QuestProgressByEtc.h and NPCDialogue.h alias these two slot IDs
            // onto their own L/R pagination buttons -- CQuestProgress itself no longer loads or
            // renders through them (RmlUi decorators now), same "kept for sibling aliasing"
            // reasoning CMyQuestInfoWindow's own IMAGE_LIST enum documents.
            IMAGE_QP_BTN_L = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN,
            IMAGE_QP_BTN_R = BITMAP_INTERFACE_QUEST_PROGRESS_BEGIN + 1,
        };

        enum
        {
            QP_WIDTH = 190,
            QP_HEIGHT = 429,
        };

        enum LOWER_VIEW_MODE
        {
            NON_PLAYER_WORDS_MODE,
            PLAYER_WORDS_MODE,
            REQUEST_REWARD_MODE
        };

    public:
        CQuestProgress();
        virtual ~CQuestProgress();
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

        void ReloadRmlTheme() override;

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

        wchar_t m_aszNPCWords[QP_NPC_LINE_MAX][QP_WORDS_ROW_MAX] = {};
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
