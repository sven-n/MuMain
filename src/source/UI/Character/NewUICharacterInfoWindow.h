// NewUICharacterInfoWindow.h: interface for the CCharacterInfoWindow class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Core/NewUIManager.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/NewUIButton.h"

namespace mu::ui::window
{
    class CCharacterInfoWindow : public CObject
    {
    public:
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

    private:
        CManager* m_pNewUIMng;
        POINT						m_Pos;

        CButton m_BtnStat[BTN_STAT_COUNT];
        CButton m_BtnExit;
        CButton m_BtnQuest;
        CButton m_BtnPet;
        CButton m_BtnMasterLevel;

    public:
        CCharacterInfoWindow();
        virtual ~CCharacterInfoWindow();
        bool Create(CManager* pNewUIMng, int x, int y);
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
    };
}
