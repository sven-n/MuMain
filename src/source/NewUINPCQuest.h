//*****************************************************************************
// File: NewUINPCQuest.h
//*****************************************************************************

#if !defined(AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_)
#define AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIMyQuestInfoWindow.h"
#include "NewUINPCShop.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    class CNewUINPCQuest : public CNewUIObj, public INewUI3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            // �⺻â
            IMAGE_NPCQUEST_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,			// newui_msgbox_back.jpg
            IMAGE_NPCQUEST_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,		// newui_item_back04.tga	(190,64)
            IMAGE_NPCQUEST_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		// newui_item_back02-l.tga	(21,320)
            IMAGE_NPCQUEST_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,	// newui_item_back02-r.tga	(21,320)
            IMAGE_NPCQUEST_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	// newui_item_back03.tga	(190,45)

            IMAGE_NPCQUEST_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_NPCQUEST_ZEN = CNewUINPCShop::IMAGE_NPCSHOP_REPAIR_MONEY,
            IMAGE_NPCQUEST_BTN_COMPLETE = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_NPCQUEST_BTN_CLOSE = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };

    private:
        enum
        {
            NPCQUEST_WIDTH = 190,
            NPCQUEST_HEIGHT = 429,
        };

        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;
        POINT					m_Pos;

        CNewUIButton			m_btnComplete;
        CNewUIButton			m_btnClose;

    public:
        CNewUINPCQuest();
        virtual ~CNewUINPCQuest();

        bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y);
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
