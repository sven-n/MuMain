//*****************************************************************************
// File: NewUIStorageInventory.h
//*****************************************************************************

#if !defined(AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_)
#define AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/NewUIButton.h"

namespace mu::ui::window
{
    class CStorageInventory : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_STORAGE_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_STORAGE_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_STORAGE_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_STORAGE_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_STORAGE_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_STORAGE_EXPAND_BTN = CMyInventory::IMAGE_INVENTORY_EXPAND_BTN,

            IMAGE_STORAGE_BTN_INSERT_ZEN = BITMAP_INTERFACE_NEW_STORAGE_BEGIN,
            IMAGE_STORAGE_BTN_TAKE_ZEN = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 1,
            IMAGE_STORAGE_BTN_UNLOCK = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 2,
            IMAGE_STORAGE_BTN_LOCK = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 3,

            IMAGE_STORAGE_MONEY = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 4,
        };

    private:
        static constexpr float STORAGE_WIDTH = 190.0f;
        static constexpr float STORAGE_HEIGHT = 429.0f;

        enum STORAGE_BUTTON
        {
            BTN_INSERT_ZEN = 0,
            BTN_TAKE_ZEN,
            BTN_LOCK,
            MAX_BTN
        };

        CManager* m_pNewUIMng;
        POINT					m_Pos;
        CButton			m_abtn[MAX_BTN];
        CButton			m_BtnExpand;
        CInventoryCtrl* m_pNewInventoryCtrl;

        bool					m_bLock;
        bool					m_bCorrectPassword;

        bool					m_bItemAutoMove;
        int						m_nBackupMouseX;
        int						m_nBackupMouseY;

        bool					m_bTakeZen;
        int						m_nBackupTakeZen;
        int						m_nBackupInvenIndex;
        int						m_nBackupSourceInvenIndex;

    public:
        CStorageInventory();
        virtual ~CStorageInventory();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 2.2f

        CInventoryCtrl* GetInventoryCtrl() const;

        bool ProcessClosing();
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        int FindEmptySlot(ITEM* pItemObj);

        bool IsStorageLocked() { return m_bLock; }
        bool IsCorrectPassword() { return m_bCorrectPassword; }
        bool IsItemAutoMove() { return m_bItemAutoMove; }

        void SetBackupTakeZen(int nZen);

        bool ProcessMyInvenItemAutoMove(CInventoryCtrl* sourceCtrl = nullptr);

        void SendRequestItemToMyInven(ITEM* pItemObj, int nStorageIndex, int nInvenIndex);

        void ProcessToReceiveStorageStatus(BYTE byStatus);
        void ProcessToReceiveStorageItems(int nIndex, std::span<const BYTE> pbyItemPacket);
        void ProcessStorageItemAutoMoveSuccess();
        void ProcessStorageItemAutoMoveFailure();

        int GetPointedItemIndex();

        void SetItemAutoMove(bool bItemAutoMove, int nSourceInvenIndex = -1);
        void SendRequestItemToStorage(ITEM* pItemObj, int nInvenIndex, int nStorageIndex);
    private:
        void LoadImages();
        void UnloadImages();

        void RenderBackImage();
        void RenderText();

        void ChangeLockBtnImage();
        void DeleteAllItems();

        void LockStorage(bool bLock);
        void SetCorrectPassword(bool bCorrectPassword)
        {
            m_bCorrectPassword = bCorrectPassword;
        }

        void InitBackupItemInfo();
        int GetBackupTakeZen() { return m_nBackupTakeZen; }
        void SetBackupInvenIndex(int nInvenIndex);
        int GetBackupInvenIndex() { return m_nBackupInvenIndex; }

        void ProcessInventoryCtrl();
        bool ProcessBtns();
        void ProcessStorageItemAutoMove();
    };
}

#endif // !defined(AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_)
