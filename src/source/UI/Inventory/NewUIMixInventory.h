// NewUIMixInventory.h: interface for the CMixInventory class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/UIControls.h"

namespace mu::ui::window
{
    class CMixInventory : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_MIXINVENTORY_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_MIXINVENTORY_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_MIXINVENTORY_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_MIXINVENTORY_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_MIXINVENTORY_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_MIXINVENTORY_MIXBTN = BITMAP_INTERFACE_NEW_MIXINVENTORY_BEGIN
        };
        enum MIX_STATE
        {
            MIX_READY = 0,
            MIX_REQUESTED,
            MIX_FINISHED
        };

    private:
        static constexpr float INVENTORY_WIDTH = 190.0f;
        static constexpr float INVENTORY_HEIGHT = 429.0f;

        CManager* m_pNewUIMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        POINT m_Pos;

        CButton m_BtnMix;

        int m_iMixState;
        int m_iMixEffectTimer;
        float m_fInventoryColor[3];
        float m_fInventoryWarningColor[3];

        CUISocketListBox m_SocketListBox;

    public:
        CMixInventory();
        virtual ~CMixInventory();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        bool ProcessMyInvenItemAutoMove(CInventoryCtrl* sourceCtrl = nullptr);
        bool ProcessMixItemAutoMoveToInventory();
        void DeleteItem(int iIndex);
        void DeleteAllItems();

        void OpeningProcess();
        bool ClosingProcess();

        void SetMixState(int iMixState);
        int GetMixState() { return m_iMixState; }

        int GetPointedItemIndex();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 3.4f

        CInventoryCtrl* GetInventoryCtrl() const;

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        bool InventoryProcess();
        bool BtnProcess();

        bool AutoMoveItem(CInventoryCtrl* srcCtrl, STORAGE_TYPE srcType,
            CInventoryCtrl* dstCtrl, STORAGE_TYPE dstType, bool requireMixSource);

        void RenderMixDescriptions(float fPos_x, float fPos_y);

        void CheckMixInventory();
        bool Mix();
        void RenderMixEffect();

        int Rtn_MixRequireZen(int _nMixZen, int _nTax);
    };
}
