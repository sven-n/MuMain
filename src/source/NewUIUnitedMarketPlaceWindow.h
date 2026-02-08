// NewUIUnitedMarketPlaceWindow.h: interface for the CNewUIUnitedMarketPlaceWindow class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "NewUIBase.h"
#include "NewUIButton.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIMyQuestInfoWindow.h"

namespace SEASON3B
{
    class CNewUIUnitedMarketPlaceWindow : public CNewUIObj, public INewUI3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_UNITEDMARKETPLACEWINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_UNITEDMARKETPLACEWINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_UNITEDMARKETPLACEWINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_UNITEDMARKETPLACEWINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_UNITEDMARKETPLACEWINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_UNITEDMARKETPLACEWINDOW_BUTTON = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
            IMAGE_UNITEDMARKETPLACEWINDOW_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_UNITEDMARKETPLACEWINDOW_BTN_CLOSE = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };
    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;
        POINT m_Pos;

        CNewUIButton m_BtnEnter;
        CNewUIButton m_BtnClose;

    public:
        CNewUIUnitedMarketPlaceWindow();
        virtual ~CNewUIUnitedMarketPlaceWindow();

        bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y);
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

        void InitButton(CNewUIButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);

        int m_iRemainTime;
        BOOL m_bIsEnterButtonLocked;
    };
}
