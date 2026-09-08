
#pragma once

#include <vector>

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "GameLogic/Quests/QuestMng.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"

namespace Rml { class ElementDocument; }

typedef std::list<DWORD> DWordList;

// Fully on RmlUi now (#panel, my_quest_info.rml/.rcss) -- retires this window's last
// CUIControl-family pieces: CUICurQuestListBox, CUIQuestContentsListBox, and 3 CButtons (exit/
// open/give-up). Tab switching, the quest list, and the selected-quest contents/reward panel are
// all RmlUi data-bound now instead of legacy list-box/button widgets. One piece stays native:
// a selected reward-item row's item-info popup (::RenderItemInfo(), a per-frame native draw, not
// a one-shot popup) -- see m_pSelectedRewardItem's own comment. That's the permanent hybrid-shape
// boundary this port lands on for that one feature, not a porting gap.
namespace mu::ui::window
{
    class CMyQuestInfoWindow : public CObject
    {
    public:
        // Kept despite this window no longer rendering any of these itself -- several sibling
        // windows (QuestProgress, QuestProgressByEtc, EmpireGuardianNPC, NPCDialogue,
        // UnitedMarketPlaceWindow, CastleWindow) alias their own IMAGE_LIST entries onto these
        // (e.g. QuestProgress.h's IMAGE_QP_LINE = CMyQuestInfoWindow::IMAGE_MYQUEST_LINE),
        // expecting LoadImages() below to have actually populated these texture slots. Real
        // cross-window texture-slot sharing, not just a name collision -- found when the build
        // broke on those other headers, not caught by this port's own research pass.
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

        // Invoked from the RmlUi document's data-event-click bindings (see Create()). Same
        // immediate-call convention as every other migrated window's RmlClickX() (see
        // CLoginMainWin::RmlClickMenu()'s header comment for why this is safe to call straight
        // into the action instead of polling a flag next frame).
        void RmlClickSelectTab(int nTab);
        void RmlClickSelectQuest(int nQuestIndex);
        void RmlClickSelectContent(int nContentIndex);
        void RmlClickOpen();
        void RmlClickGiveUp();
        void RmlClickExit();

    private:
        // Populates the shared texture slots IMAGE_LIST names -- kept solely because sibling
        // windows alias onto these slot indices (see IMAGE_LIST's own comment above); this
        // window's own rendering no longer reads any of them.
        void LoadImages();
        void UnloadImages();

        // One quest-list row -- mirrors SCurQuestItem (UIControls.h), minus the legacy list box's
        // own scroll/render bookkeeping.
        struct QuestEntry
        {
            Rml::String text;
            int index = 0; // the real DWORD quest index (LOWORD=questNumber, HIWORD=questGroup),
                            // narrowed to int -- quest indices are small packed values, same
                            // assumption ServerSelWin's own index/btnPos fields already make.
            bool selected = false;
        };
        // One quest-contents/reward row -- mirrors SQuestContents (UIControls.h).
        struct ContentEntry
        {
            Rml::String text;
            Rml::String color; // "rgba(r,g,b,a)"
            bool bold = false;
            int index = 0; // position within m_ContentRows -- passed back to RmlClickSelectContent
            bool clickable = false; // true for a reward/request-item row (data-event-click target)
        };
        // A single text line -- reused for every multi-line message block (empty-quest message,
        // job-change contents/state) since they're all the same "list of lines" shape.
        struct TextLine
        {
            Rml::String text;
        };

        void SyncRmlModel();
        static std::vector<TextLine> BuildTextLines(int nGlobalTextIndex, int nPixelWidth);

        struct MyQuestInfoRmlModel
        {
            // Shared transform group -- this window is movable (SetPos()), not HUD-anchored, so
            // this is sourced from UI::Scaling::GetActiveTransform() the same way
            // MyInventoryRmlModel::rootX's own header comment describes.
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;

            int activeTab = TAB_QUEST;
            Rml::String tabQuestLabel, tabJobChangeLabel, tabCastleTempleLabel;

            bool questListEmpty = true;
            std::vector<TextLine> emptyQuestLines;
            std::vector<QuestEntry> quests;
            std::vector<ContentEntry> contents;

            bool openEnabled = false;
            bool giveupEnabled = false;

            // Static, set once at Create() like the tab labels above -- CButton never had these
            // (legacy relied on the player already knowing the icons), a straightforward gap to
            // close now that RmlUi makes a hover tooltip this cheap (see my_inventory.rcss's own
            // #btn_exit/.tooltip-exit precedent this reuses verbatim).
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

        // Raw content-row data kept alongside the model's own display-only ContentEntry list, so
        // click handling (reward-item selection) has the same fields
        // CUIQuestContentsListBox::RenderDataLine/RenderCoveredInterface (UIControls.cpp) used.
        struct ContentRowData
        {
            Rml::String text;
            DWORD dwColor = 0;
            DWORD dwType = 0;
            ITEM* pItem = nullptr;
        };
        std::vector<ContentRowData> m_ContentRows;

        // Selected reward-item row's live item-info popup -- the one piece of this window still
        // native-rendered every frame (::RenderItemInfo(), Engine/Object/ZzzInventory.h), matching
        // CUIQuestContentsListBox::RenderCoveredInterface's original per-frame-while-selected
        // behavior. Position is a fixed anchor near the contents panel rather than the original's
        // exact selected-row Y (which needed the legacy list box's own scroll/line-height math) --
        // a deliberate simplification, not a correctness requirement.
        ITEM* m_pSelectedRewardItem = nullptr;
    };
}
