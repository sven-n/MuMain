
#pragma once

#include <vector>

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "GameLogic/Quests/QuestMng.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Quests/QuestRewardModel.h"

namespace Rml { class ElementDocument; }

typedef std::list<DWORD> DWordList;

// Fully RmlUi-based (#panel, my_quest_info.rml/.rcss), except the selected reward item's
// info popup, which stays a native per-frame draw -- see m_pSelectedRewardItem.
namespace mu::ui::window
{
    class CMyQuestInfoWindow : public CObject
    {
    public:
        // Kept even though this window no longer renders them -- sibling windows (QuestProgress,
        // QuestProgressByEtc, EmpireGuardianNPC, NPCDialogue, etc.) alias their own IMAGE_LIST
        // entries onto these slots and expect LoadImages() below to have populated them.
        enum IMAGE_LIST
        {
            IMAGE_MYQUEST_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
            IMAGE_MYQUEST_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,		//. newui_item_back01.tga	(190,64)
            IMAGE_MYQUEST_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
            IMAGE_MYQUEST_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
            IMAGE_MYQUEST_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)
            IMAGE_MYQUEST_BTN_EXIT = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,	//. newui_exit_00.tga

            IMAGE_MYQUEST_LINE = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN,
            IMAGE_MYQUEST_BTN_OPEN = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN + 1,
            IMAGE_MYQUEST_BTN_GIVE_UP = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN + 2,
            IMAGE_MYQUEST_TAB_BACK = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN + 3,
            IMAGE_MYQUEST_TAB_SMALL = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN + 4,
            IMAGE_MYQUEST_TAB_BIG = BITMAP_INTERFACE_MYQUEST_WINDOW_BEGIN + 5,
        };
        enum
        {
            MYQUESTINFO_WINDOW_WIDTH = 190,
            MYQUESTINFO_WINDOW_HEIGHT = 429,
        };

        enum TAB_BUTTON_INDEX
        {
            TAB_NON = -1,
            TAB_QUEST = 0,
            TAB_JOB_CHANGE,
            TAB_CASTLE_TEMPLE,
        };

    public:
        CMyQuestInfoWindow();
        virtual ~CMyQuestInfoWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void Show(bool bShow) override;

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 3.3f

        void OpenningProcess();
        void ClosingProcess();

        void UnselectQuestList();
        void SetCurQuestList(DWordList* pDWordList);
        void SetSelQuestSummary();
        void SetSelQuestRequestReward();

        void QuestOpenBtnEnable(bool bEnable);
        void QuestGiveUpBtnEnable(bool bEnable);

        DWORD GetSelQuestIndex();

        // Invoked directly from RmlUi data-event-click bindings (see Create()), not polled.
        void RmlClickSelectTab(int nTab);
        void RmlClickSelectQuest(int nQuestIndex);
        void RmlClickSelectContent(int nContentIndex);
        void RmlClickOpen();
        void RmlClickGiveUp();
        void RmlClickExit();

        void ReloadRmlTheme();

    private:
        // Populates the shared IMAGE_LIST texture slots sibling windows alias onto (see IMAGE_LIST above).
        void LoadImages();
        void UnloadImages();
        void BuildRmlUi();

        // One quest-list row; mirrors SCurQuestItem (UIControls.h) minus legacy scroll/render bookkeeping.
        struct QuestEntry
        {
            Rml::String text;
            int index = 0; // real DWORD quest index (LOWORD=questNumber, HIWORD=questGroup), narrowed to int
            bool selected = false;
        };
        // One quest-contents/reward row -- mirrors SQuestContents (UIControls.h).
        struct ContentEntry
        {
            Rml::String text;
            Rml::String style; // UI::Quests::RewardModel::StyleKey()
            bool bold = false;
            int index = 0; // position within m_ContentRows -- passed back to RmlClickSelectContent
            bool clickable = false; // true for a reward/request-item row (data-event-click target)
        };
        // A single text line, reused for every multi-line message block (same "list of lines" shape).
        struct TextLine
        {
            Rml::String text;
        };

        void SyncRmlModel();
        static std::vector<TextLine> BuildTextLines(int nGlobalTextIndex, int nPixelWidth);

        struct MyQuestInfoRmlModel
        {
            // Movable window (SetPos()), not HUD-anchored -- sourced from UI::Scaling::GetActiveTransform().
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

            int activeTab = TAB_QUEST;
            Rml::String tabQuestLabel, tabJobChangeLabel, tabCastleTempleLabel;

            bool questListEmpty = true;
            std::vector<TextLine> emptyQuestLines;
            std::vector<QuestEntry> quests;
            std::vector<ContentEntry> contents;

            bool openEnabled = false;
            bool giveupEnabled = false;

            // Set once at Create(); legacy CButton had no tooltips here, RmlUi makes them cheap to add.
            Rml::String openTooltip, giveupTooltip, exitTooltip;

            Rml::String jobChangeTitle;
            std::vector<TextLine> jobChangeLines;
            std::vector<TextLine> jobChangeStateLines;

            Rml::String castleTitle, castleLine0, castleLine1;
            Rml::String templeTitle, templeLine0, templeLine1;
        };
        RmlModelBinder<MyQuestInfoRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

    private:
        CManager* m_pNewUIMng;
        POINT m_Pos;

        TAB_BUTTON_INDEX m_eTabBtnIndex;

        DWORD m_dwSelectedQuestIndex = 0;
        std::vector<DWORD> m_QuestIndices; // last SetCurQuestList() data, kept for SyncRmlModel()

        // Raw content-row data for click handling, alongside the model's display-only ContentEntry
        // list. UI::Quests::RewardModel::RowData (not a private struct here) since
        // SetSelQuestRequestReward() below builds these via the shared reward-row helper, same one
        // CQuestProgress/CQuestProgressByEtc use for their own reward list.
        std::vector<UI::Quests::RewardModel::RowData> m_ContentRows;

        // Selected reward-item's info popup -- still native-rendered every frame (::RenderItemInfo())
        // rather than ported to RmlUi. Anchored near the panel, not at the exact row Y.
        ITEM* m_pSelectedRewardItem = nullptr;
    };
}
