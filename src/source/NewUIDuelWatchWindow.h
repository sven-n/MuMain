// NewUIDuelWatchWindow.h: interface for the CNewUIDuelWatchWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NewUIBase.h"
#include "NewUIButton.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIMyQuestInfoWindow.h"

namespace SEASON3B
{
    class CNewUIDuelWatchWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_DUELWATCHWINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_DUELWATCHWINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_DUELWATCHWINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_DUELWATCHWINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_DUELWATCHWINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_DUELWATCHWINDOW_BUTTON = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
            IMAGE_DUELWATCHWINDOW_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
        };
    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;

        CNewUIButton m_BtnChannel[4];
        BOOL m_bChannelEnable[4];
    public:
        CNewUIDuelWatchWindow();
        virtual ~CNewUIDuelWatchWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        void OpeningProcess();
        void ClosingProcess();

        float GetLayerDepth();	//. 5.0f

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        bool BtnProcess();

        void InitButton(CNewUIButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);
    };
}
