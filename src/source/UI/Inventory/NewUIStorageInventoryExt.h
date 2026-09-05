//*****************************************************************************
// File: NewUIStorageInventory.h
//*****************************************************************************

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"

namespace mu::ui::window
{
    class CStorageInventoryExt : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_STORAGE_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_STORAGE_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_STORAGE_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_STORAGE_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_STORAGE_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_INVENTORY_EXIT_BTN = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };

    private:
        static constexpr float STORAGE_WIDTH = 190.0f;
        static constexpr float STORAGE_HEIGHT = 429.0f;

        CManager* m_pNewUIMng;
        POINT					m_Pos;

        CInventoryCtrl* m_pNewInventoryCtrl;
        CButton m_BtnExit;

        bool					m_bItemAutoMove;
        int						m_nBackupMouseX;
        int						m_nBackupMouseY;
        int						m_nBackupSourceInvenIndex;

    public:
        CStorageInventoryExt();
        ~CStorageInventoryExt() override;

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent() override;
        bool UpdateKeyEvent() override;
        bool Update() override;
        bool Render() override;

        float GetLayerDepth() override;	//. 2.2f

        CInventoryCtrl* GetInventoryCtrl() const;

        bool ProcessClosing() const;
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const;
        int FindEmptySlot(const ITEM* pItemObj) const;
        bool ProcessMyInvenItemAutoMove(CInventoryCtrl* sourceCtrl = nullptr);

        bool IsItemAutoMove() const { return m_bItemAutoMove; }

        void ProcessToReceiveStorageItems(int nIndex, std::span<const BYTE> pbyItemPacket);
        void ProcessStorageItemAutoMoveSuccess();
        void ProcessStorageItemAutoMoveFailure();

        int GetPointedItemIndex() const;

        void SetItemAutoMove(bool bItemAutoMove, int nSourceInvenIndex = -1);

    private:
        void LoadImages() const;
        void UnloadImages();

        void RenderBackImage() const;
        void RenderText() const;

        void DeleteAllItems() const;

        void ProcessInventoryCtrl();
        bool ProcessBtns() const;
        void ProcessStorageItemAutoMove();
    };
}
