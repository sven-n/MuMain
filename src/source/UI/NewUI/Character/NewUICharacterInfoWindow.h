// NewUICharacterInfoWindow.h: interface for the CNewUICharacterInfoWindow class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/NewUI/NewUIManager.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/NewUI/Inventory/NewUIMyInventory.h"
#include "UI/NewUI/Widgets/NewUIButton.h"

namespace SEASON3B
{
    class CNewUICharacterInfoWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_CHAINFO_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
            IMAGE_CHAINFO_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,		// newui_item_back04.tga	(190,64)
            IMAGE_CHAINFO_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
            IMAGE_CHAINFO_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
            IMAGE_CHAINFO_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)

            IMAGE_CHAINFO_TABLE_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
            IMAGE_CHAINFO_TABLE_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
            IMAGE_CHAINFO_TABLE_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
            IMAGE_CHAINFO_TABLE_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_CHAINFO_TABLE_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_CHAINFO_TABLE_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_CHAINFO_TABLE_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)

            IMAGE_CHAINFO_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,

            IMAGE_CHAINFO_TEXTBOX = BITMAP_INTERFACE_NEW_CHAINFO_WINDOW_BEGIN,
            IMAGE_CHAINFO_BTN_STAT,
            IMAGE_CHAINFO_BTN_QUEST,
            IMAGE_CHAINFO_BTN_PET,
            IMAGE_CHAINFO_BTN_MASTERLEVEL,
            IMAGE_CHAINFO_BTN_SUGGEST,
        };

        enum
        {
            CHAINFO_WINDOW_WIDTH = 190,
            // 429 in the original layout; 8px taller to host the stat-point
            // suggestion bar between the last stat row and the button strip.
            CHAINFO_WINDOW_HEIGHT = 437,
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
            HEIGHT_SUGGEST_BAR = 374,
            HEIGHT_BOTTOM_BTN = 400,
            SUGGEST_PRESET_MAX = 4,
        };

        // One stat-point build recommendation: a weight per stat. The weights
        // are relative, not absolute - the free points are split between the
        // stats proportionally to them.
        struct SUGGEST_PRESET
        {
            const wchar_t* const* s_pNameSlot;
            int s_aWeight[BTN_STAT_COUNT];
        };

    private:
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;

        CNewUIButton m_BtnStat[BTN_STAT_COUNT];
        CNewUIButton m_BtnExit;
        CNewUIButton m_BtnQuest;
        CNewUIButton m_BtnPet;
        CNewUIButton m_BtnMasterLevel;
        CNewUIButton m_BtnSuggestPreset;
        CNewUIButton m_BtnSuggestApply;

        // 0 = suggestion off, otherwise 1-based index into the class preset table.
        int m_iSuggestPreset;
        CLASS_TYPE m_eSuggestPresetClass;
        int m_aSuggestPoint[BTN_STAT_COUNT];

    public:
        CNewUICharacterInfoWindow();
        virtual ~CNewUICharacterInfoWindow();
        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();
        void SetPos(int x, int y);
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        float GetLayerDepth();	//. 5.1f
        void OpenningProcess();

    private:
        void LoadImages();
        void UnloadImages();
        void ResetEquipmentLevel();
        void SetButtonInfo();
        bool BtnProcess();
        void RenderFrame();
        void RenderTexts();
        void RenderSubjectTexts();
        void RenderTableTexts();
        void RenderAttribute();
        void RenderButtons();
        void RenderSuggestion();

        int GetStatCount() const;
        const SUGGEST_PRESET* GetPresetTable(int& iCount) const;
        void ResetSuggestion();
        void CycleSuggestPreset();
        void CalcSuggestion();
        void ApplySuggestedPoints();
    };
}
