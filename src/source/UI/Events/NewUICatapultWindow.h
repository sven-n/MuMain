// NewUICatapultWindow.h: interface for the CCatapultWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICATAPULTWINDOW_H__064BC38C_5F26_4003_A6C7_7270A11DEEBF__INCLUDED_)
#define AFX_NEWUICATAPULTWINDOW_H__064BC38C_5F26_4003_A6C7_7270A11DEEBF__INCLUDED_

#pragma once

#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/NewUIButton.h"

namespace mu::ui::window
{
    class CCatapultWindow : public CObject
    {
    private:

        class CCatapultGroupButton
        {
        public:
            CCatapultGroupButton();
            virtual ~CCatapultGroupButton();

            void Create(int iType, POINT ptWindow);
            int UpdateMouseEvent();
            void Render();
            int GetIndex();

        private:
            void Initialize();
            void AllUnLock();
            void BtnSelected(int iIndex);

            CButton* m_pButton;

            int m_iBtnNum;
            int m_iType;
            int m_iIndex;
        };

    public:
        enum IMAGE_LIST
        {
            // 기본창
            IMAGE_CATAPULT_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
            IMAGE_CATAPULT_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,		// newui_item_back01.tga	(190,64)
            IMAGE_CATAPULT_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
            IMAGE_CATAPULT_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
            IMAGE_CATAPULT_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)
            IMAGE_CATAPULT_BTN_EXIT = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,	//. newui_exit_00.tga

            // 테이블
            IMAGE_CATAPULT_TABLE_TOP_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
            IMAGE_CATAPULT_TABLE_TOP_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
            IMAGE_CATAPULT_TABLE_BOTTOM_LEFT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
            IMAGE_CATAPULT_TABLE_BOTTOM_RIGHT = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_CATAPULT_TABLE_TOP_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_CATAPULT_TABLE_BOTTOM_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_CATAPULT_TABLE_LEFT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_CATAPULT_TABLE_RIGHT_PIXEL = CInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)

            // 버튼
            IMAGE_CATAPULT_BTN_FIRE = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,

            IMAGE_CATAPULT_BTN_SMALL = BITMAP_CATAPULT_BEGIN,
            IMAGE_CATAPULT_BTN_BIG,
        };
        enum CATAPULT_TYPE
        {
            CATAPULT_ATTACK = 1,
            CATAPULT_DEFENSE = 2,
        };

    public:
        CCatapultWindow();
        virtual ~CCatapultWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 5.0f

        void OpenningProcess();
        void ClosingProcess();

        void Init(int iKey, int iType);
        void DoFire(int iKey, int iResult, int iType, int iPositionX, int iPositionY);
        void DoFireFixStartPosition(int iType, int iPositionX, int iPositionY);
        void SetCameraPos(float x = 0.f, float y = 0.f, float z = 0.f);
        void GetCameraPos(vec3_t& vPos);

    private:
        void LoadImages();
        void UnloadImages();

        void SetButtonInfo();

        bool BtnProcess();

        void RenderFrame();
        void RenderTexts();
        void RenderButtons();

        void RenderOutlineUpper(float fPos_x, float fPos_y, float fWidth, float fHeight);
        void RenderOutlineLower(float fPos_x, float fPos_y, float fWidth, float fHeight);

    private:
        CManager* m_pNewUIMng;
        POINT m_Pos;

        CButton m_BtnExit;
        CCatapultGroupButton m_BtnChoiceArea;	// 지역선택
        CButton m_BtnFire;	// 발사버튼

        int m_iType;
        int m_iNpcKey;
        vec3_t m_vCameraPos;
    };
}

#endif // !defined(AFX_NEWUICATAPULTWINDOW_H__064BC38C_5F26_4003_A6C7_7270A11DEEBF__INCLUDED_)
