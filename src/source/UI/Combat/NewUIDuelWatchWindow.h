// NewUIDuelWatchWindow.h: interface for the CDuelWatchWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Widgets/NewUIButton.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Quests/NewUIMyQuestInfoWindow.h"

namespace mu::ui::window
{
    class CDuelWatchWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_DUELWATCHWINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_DUELWATCHWINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_DUELWATCHWINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_DUELWATCHWINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_DUELWATCHWINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_DUELWATCHWINDOW_BUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
            IMAGE_DUELWATCHWINDOW_LINE = CMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
        };
    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        POINT m_Pos;

        CButton m_BtnChannel[4];
        BOOL m_bChannelEnable[4];
    public:
        CDuelWatchWindow();
        virtual ~CDuelWatchWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
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

        void InitButton(CButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);
    };
}
