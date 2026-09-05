// CEmpireGuardianNPC.h: interface for the CEmpireGuardianNPC class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Core/NewUIManager.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Quests/NewUIMyQuestInfoWindow.h"

namespace mu::ui::window
{
    class CEmpireGuardianNPC : public CObject, public I3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_EMPIREGUARDIAN_NPC_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_EMPIREGUARDIAN_NPC_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_EMPIREGUARDIAN_NPC_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_EMPIREGUARDIAN_NPC_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_EMPIREGUARDIAN_NPC_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_EMPIREGUARDIAN_NPC_LINE = CMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
            IMAGE_EMPIREGUARDIAN_NPC_BTN = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
        };

    private:
        enum EMPIREGUARDIAN_TIME_WINDOW_SIZE
        {
            NPC_WINDOW_WIDTH = 190,
            NPC_WINDOW_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        C3DRenderMng* m_pNewUI3DRenderMng;

        POINT				m_Pos;
        CButton		m_btPositive;
        CButton		m_btNegative;
        bool				m_bCanClick;

    public:
        CEmpireGuardianNPC();
        virtual ~CEmpireGuardianNPC();

        bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y);
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
        void InitButton(CButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption);
        void LoadImages();
        void UnloadImages();

        bool BtnProcess();
        void RenderFrame();
        void RenderItem3D();
    };
}
