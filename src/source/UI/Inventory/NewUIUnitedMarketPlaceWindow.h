// NewUIUnitedMarketPlaceWindow.h: interface for the CUnitedMarketPlaceWindow class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Widgets/NewUIButton.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Quests/NewUIMyQuestInfoWindow.h"

namespace mu::ui::window
{
    class CUnitedMarketPlaceWindow : public CObject, public I3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_UNITEDMARKETPLACEWINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_UNITEDMARKETPLACEWINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_UNITEDMARKETPLACEWINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_UNITEDMARKETPLACEWINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_UNITEDMARKETPLACEWINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_UNITEDMARKETPLACEWINDOW_BUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
            IMAGE_UNITEDMARKETPLACEWINDOW_LINE = CMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_UNITEDMARKETPLACEWINDOW_BTN_CLOSE = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };
    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        C3DRenderMng* m_pNewUI3DRenderMng;
        POINT m_Pos;

        CButton m_BtnEnter;
        CButton m_BtnClose;

    public:
        CUnitedMarketPlaceWindow();
        virtual ~CUnitedMarketPlaceWindow();

        bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void Render3D();

        bool IsVisible() const;

        void OpeningProcess();
        void ClosingProcess();

        float GetLayerDepth();	//. 5.0f

        void SetRemainTime(int iTime);
        void LockEnterButton(BOOL bLock);

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        bool BtnProcess();
        void RenderItem3D();

        void InitButton(CButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);

        int m_iRemainTime;
        BOOL m_bIsEnterButtonLocked;
    };
}
