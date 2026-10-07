#pragma once

#include <vector>

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"

namespace Rml { class ElementDocument; }

// Fully RmlUi-based (#panel, character_info.rml/.rcss) -- no permanently-native content (no live
// 3D icon, no CUITextInputBox), unlike the inventory family it shares frame
// assets with.
namespace mu::ui::window
{
    class CCharacterInfoWindow : public CObject
    {
    public:
        // Kept even though this window no longer renders through the legacy bitmap-atlas system --
        // CGensRanking (IMAGE_RANKBACK_TEXTBOX) aliases its own IMAGE_LIST entries onto these same
        // texture slots and expects LoadImages() below to have populated them (same reason
        // CMyQuestInfoWindow keeps its own IMAGE_LIST).
        enum IMAGE_LIST
        {
            IMAGE_CHAINFO_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
            IMAGE_CHAINFO_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP2,		// newui_item_back04.tga	(190,64)
            IMAGE_CHAINFO_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
            IMAGE_CHAINFO_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
            IMAGE_CHAINFO_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)

            IMAGE_CHAINFO_TABLE_TOP_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
            IMAGE_CHAINFO_TABLE_TOP_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
            IMAGE_CHAINFO_TABLE_BOTTOM_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
            IMAGE_CHAINFO_TABLE_BOTTOM_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_CHAINFO_TABLE_TOP_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_CHAINFO_TABLE_LEFT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_CHAINFO_TABLE_RIGHT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)

            IMAGE_CHAINFO_BTN_EXIT = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,

            IMAGE_CHAINFO_TEXTBOX = BITMAP_INTERFACE_NEW_CHAINFO_WINDOW_BEGIN,
            IMAGE_CHAINFO_BTN_STAT,
            IMAGE_CHAINFO_BTN_QUEST,
            IMAGE_CHAINFO_BTN_PET,
            IMAGE_CHAINFO_BTN_MASTERLEVEL,
        };

        enum
        {
            CHAINFO_WINDOW_WIDTH = 190,
            CHAINFO_WINDOW_HEIGHT = 429,
            HEIGHT_STRENGTH = 120,
            HEIGHT_DEXTERITY = 175,
            HEIGHT_VITALITY = 240,
            HEIGHT_ENERGY = 295,
            HEIGHT_CHARISMA = 350,
            BTN_STAT_COUNT = 5,
            STAT_STRENGTH = 0,
            STAT_DEXTERITY,
            STAT_VITALITY,
            STAT_ENERGY,
            STAT_CHARISMA,
        };

    public:
        CCharacterInfoWindow();
        virtual ~CCharacterInfoWindow();
        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();
        void SetPos(int x, int y);
        Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
        void Show(bool bShow) override;
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        float GetLayerDepth();	//. 5.1f
        void OpenningProcess();

        // Invoked directly from RmlUi data-event-click bindings (see Create()), not polled.
        void RmlClickIncreaseStat(int stat);
        void RmlClickExit();
        void RmlClickQuest();
        void RmlClickPet();
        void RmlClickMasterLevel();

    private:
        // Populates the shared IMAGE_LIST texture slots sibling windows alias onto (see IMAGE_LIST above).
        void LoadImages();
        void UnloadImages();
        void ResetEquipmentLevel();

        // One derived-stat display line below an attribute's label/value row (attack, defense,
        // attack-speed, mana, magic/curse damage, class-specific bonus lines, etc.) -- count and
        // order vary per class, so these stack via normal block flow rather than fixed per-line
        // offsets (see character_info.rcss's header comment).
        struct StatLine
        {
            Rml::String text;
            // Why the row reads the way it does: "base", "boosted" by an item or set, or
            // "buffed" by a consumable. Each theme's character_info.rcss owns the colours.
            Rml::String source;
        };

        void BuildRmlUi();
        void SyncRmlModel();
        void BuildSubjectTexts();
        void BuildTableTexts();
        void BuildAttributeLines();

        struct CharacterInfoRmlModel
        {
            // Movable window (SetPos(), collision-shuffled by PanelColumnX), not HUD-anchored --
            // sourced from UI::Scaling::GetActiveTransform(), same convention as my_quest_info.
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            float panelWidth = 190.f;
            float textPx = 0.f; // UI::RmlBridge::SyncNativeTextSize()

            bool canLevelUp = false;   // CharacterAttribute->LevelUpPoint > 0
            bool showCharisma = false; // base class == CLASS_DARK_LORD

            // CHARACTER::PK, not a colour: the theme decides what each outlaw level looks like.
            Rml::String nameText;
            int pkLevel = 0;
            Rml::String classNameText, serverNameText;
            float classNameOpacity = 1.f, serverNameOpacity = 0.f;

            Rml::String levelText;
            Rml::String levelUpPointText;
            Rml::String expText;
            Rml::String pointProbabilityText;
            Rml::String pointText;

            Rml::String strLabel, agiLabel, vitLabel, eneLabel, cmdLabel;
            // *ValueSource is where the attribute's current total comes from -- "potion" while a
            // Secret Potion is up, "boosted" when items or a buff add to it, "base" otherwise.
            // What each of the three looks like is the theme's to decide.
            Rml::String strValueText, strValueSource;
            Rml::String agiValueText, agiValueSource;
            Rml::String vitValueText, vitValueSource;
            Rml::String eneValueText, eneValueSource;
            Rml::String cmdValueText, cmdValueSource;

            std::vector<StatLine> strLines;
            std::vector<StatLine> agiLines;
            std::vector<StatLine> vitLines;
            std::vector<StatLine> eneLines;

            bool masterLevelEnabled = false;

            // Set once at Create() -- static tooltip text, no per-frame update needed.
            Rml::String exitTooltip, questTooltip, petTooltip, masterLevelTooltip;
        };

        void BindRmlModel(Rml::DataModelConstructor& c, CharacterInfoRmlModel& model);
        UI::RmlBridge::ThemedView<CharacterInfoRmlModel> m_RmlView{"character_info",
            [this](Rml::DataModelConstructor& c, CharacterInfoRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/character_info.rml"}}};

    private:
        CManager* m_pNewUIMng;
        POINT m_Pos;
    };
}
