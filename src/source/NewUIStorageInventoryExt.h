//*****************************************************************************
// File: NewUIStorageInventory.h
//*****************************************************************************

#pragma once

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"

namespace SEASON3B
{
    class CNewUIStorageInventoryExt : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_STORAGE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_STORAGE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_STORAGE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_STORAGE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_STORAGE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_INVENTORY_EXIT_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        };

    private:
        enum
        {
            STORAGE_WIDTH = 190,
            STORAGE_HEIGHT = 429,
        };

        CNewUIManager* m_pNewUIMng;
        POINT					m_Pos;

        CNewUIInventoryCtrl* m_pNewInventoryCtrl;
        CNewUIButton m_BtnExit;

        bool					m_bItemAutoMove;
        int						m_nBackupMouseX;
        int						m_nBackupMouseY;

    public:
        CNewUIStorageInventoryExt();
        ~CNewUIStorageInventoryExt() override;

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent() override;
        bool UpdateKeyEvent() override;
        bool Update() override;
        bool Render() override;

        float GetLayerDepth() override;	//. 2.2f

        CNewUIInventoryCtrl* GetInventoryCtrl() const;

        bool ProcessClosing() const;
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const;
        int FindEmptySlot(const ITEM* pItemObj) const;

        bool IsItemAutoMove() const { return m_bItemAutoMove; }

        void ProcessToReceiveStorageItems(int nIndex, std::span<const BYTE> pbyItemPacket);
        void ProcessStorageItemAutoMoveSuccess();
        void ProcessStorageItemAutoMoveFailure();

        int GetPointedItemIndex() const;

        void SetItemAutoMove(bool bItemAutoMove);

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
