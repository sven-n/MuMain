//*****************************************************************************
// File: NewUINPCQuest.h
//*****************************************************************************

#if !defined(AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_)
#define AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Quests/NewUIMyQuestInfoWindow.h"
#include "UI/NPCs/NewUINPCShop.h"
#include "UI/Widgets/NewUIButton.h"

namespace mu::ui::window
{
    class CNPCQuest : public CObject, public I3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            // 기본창
            IMAGE_NPCQUEST_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
            IMAGE_NPCQUEST_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP2,		// newui_item_back04.tga	(190,64)
            IMAGE_NPCQUEST_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
            IMAGE_NPCQUEST_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
            IMAGE_NPCQUEST_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)

            IMAGE_NPCQUEST_LINE = CMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_NPCQUEST_ZEN = CNPCShop::IMAGE_NPCSHOP_REPAIR_MONEY,
            IMAGE_NPCQUEST_BTN_COMPLETE = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_NPCQUEST_BTN_CLOSE = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };

    private:
        enum
        {
            NPCQUEST_WIDTH = 190,
            NPCQUEST_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        C3DRenderMng* m_pNewUI3DRenderMng;
        POINT					m_Pos;

        CButton			m_btnComplete;
        CButton			m_btnClose;

    public:
        CNPCQuest();
        virtual ~CNPCQuest();

        bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void Render3D();

        bool IsVisible() const;

        float GetLayerDepth();	//. 3.1f

        void ProcessOpening();
        bool ProcessClosing();

    private:
        void LoadImages();
        void UnloadImages();

        bool UpdateSelTextMouseEvent();
        void RenderBackImage();
        void RenderText();
        bool RenderItemMobText();
        void RenderItem3D();

        bool ProcessBtns();
    };
}

#endif // !defined(AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_)
