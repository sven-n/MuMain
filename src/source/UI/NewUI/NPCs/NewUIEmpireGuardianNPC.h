// CNewUIEmpireGuardianNPC.h: interface for the CNewUIEmpireGuardianNPC class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/NewUI/NewUIManager.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/NewUI/Inventory/NewUIMyInventory.h"
#include "UI/NewUI/Quests/NewUIMyQuestInfoWindow.h"

namespace SEASON3B
{
    class CNewUIEmpireGuardianNPC : public CNewUIObj, public INewUI3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_EMPIREGUARDIAN_NPC_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_EMPIREGUARDIAN_NPC_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_EMPIREGUARDIAN_NPC_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_EMPIREGUARDIAN_NPC_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_EMPIREGUARDIAN_NPC_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_EMPIREGUARDIAN_NPC_LINE = CNewUIMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_EMPIREGUARDIAN_NPC_BTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
        };

    private:
        enum EMPIREGUARDIAN_TIME_WINDOW_SIZE
        {
            NPC_WINDOW_WIDTH = 190,
            NPC_WINDOW_HEIGHT = 429,
        };

        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;

        POINT				m_Pos;
        CNewUIButton		m_btPositive;
        CNewUIButton		m_btNegative;
        bool				m_bCanClick;

    public:
        CNewUIEmpireGuardianNPC();
        virtual ~CNewUIEmpireGuardianNPC();

        bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void Render3D();

        float GetLayerDepth();	//. 1.2f

        bool IsVisible() const;

        void OpenningProcess();
        void ClosingProcess();

    private:
        void InitButton(CNewUIButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);
        void LoadImages();
        void UnloadImages();

        bool BtnProcess();
        void RenderFrame();
        void RenderItem3D();
    };
}
