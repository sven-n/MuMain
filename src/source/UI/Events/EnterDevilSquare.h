
#ifndef _ENTERDEVILSQUARE_H_
#define _ENTERDEVILSQUARE_H_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Widgets/Window/Button.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Events/EventEntryView.h"

namespace mu::ui::window
{
    class CEnterDevilSquare : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            // Base Window (Reference)
            IMAGE_ENTERDS_BASE_WINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
            IMAGE_ENTERDS_BASE_WINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,			//. newui_item_back01.tga	(190,64)
            IMAGE_ENTERDS_BASE_WINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,			//. newui_item_back02-l.tga	(21,320)
            IMAGE_ENTERDS_BASE_WINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_ENTERDS_BASE_WINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,		//. newui_item_back03.tga	(190,45)
            IMAGE_ENTERDS_BASE_WINDOW_BTN_EXIT = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,		//. newui_exit_00.tga

            IMAGE_ENTERDS_BASE_WINDOW_BTN_ENTER = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_BIG	//. newui_btn_empty_big.tga	(180, 87)
        };

    private:
        enum ENTERDS_WINDOW_SIZE
        {
            ENTERDS_BASE_WINDOW_WIDTH = 190,
            ENTERDS_BASE_WINDOW_HEIGHT = 429,
        };

        enum ENTERDS_ENTERBTN_STATE
        {
            ENTERBTN_DISABLE = 0,
            ENTERBTN_ENABLE,
        };

        enum
        {
            ENTER_BTN_VAL = 33,			// 버튼 사이의 간격

            MAX_ENTER_GRADE = 7,
        };

    private:
        CManager* m_pNewUIMng;
        POINT						m_Pos;

        // The window's RmlUi document (entry frame, lines, level buttons, exit).
        EventEntryView m_View;

        int							m_iDevilSquareLimitLevel[MAX_ENTER_GRADE * 2][2];
        int m_iNumActiveBtn; // the enabled button: the hero's level band

    public:
        CEnterDevilSquare();
        virtual ~CEnterDevilSquare();

        bool Create(CManager* pNewUIMng, int x, int y);
        Rml::ElementDocument* GetFillDocument() const override { return m_View.Document(); }
        Rml::ElementDocument* GetPlacedDocument() const override { return m_View.Document(); }
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();


        float GetLayerDepth();	//. 4.0f

        void OpenningProcess();
        void ClosingProcess();


    private:
        // The title and description lines of the original's Render(), with the level buttons.
        void SetViewContent(const std::vector<EventEntryView::Button>& buttons);

        int	CheckLimitLV(int iIndex);
    };
}

#endif // _ENTERDEVILSQUARE_H_
