//*****************************************************************************
// File: NewUIStorageInventory.h
//*****************************************************************************

#if !defined(AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_)
#define AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    class CNewUIStorageInventory : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_STORAGE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_STORAGE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_STORAGE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_STORAGE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_STORAGE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_STORAGE_EXPAND_BTN = CNewUIMyInventory::IMAGE_INVENTORY_EXPAND_BTN,

            IMAGE_STORAGE_BTN_INSERT_ZEN = BITMAP_INTERFACE_NEW_STORAGE_BEGIN,
            IMAGE_STORAGE_BTN_TAKE_ZEN = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 1,
            IMAGE_STORAGE_BTN_UNLOCK = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 2,
            IMAGE_STORAGE_BTN_LOCK = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 3,

            IMAGE_STORAGE_MONEY = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 4,
        };

    private:
        enum
        {
            STORAGE_WIDTH = 190,
            STORAGE_HEIGHT = 429,
        };

        enum STORAGE_BUTTON
        {
            BTN_INSERT_ZEN = 0,
            BTN_TAKE_ZEN,
            BTN_LOCK,
            MAX_BTN
        };

        CNewUIManager* m_pNewUIMng;
        POINT					m_Pos;
        CNewUIButton			m_abtn[MAX_BTN];
        CNewUIButton			m_BtnExpand;
        CNewUIInventoryCtrl* m_pNewInventoryCtrl;

        bool					m_bLock;
        bool					m_bCorrectPassword;

        bool					m_bItemAutoMove;
        int						m_nBackupMouseX;
        int						m_nBackupMouseY;

        bool					m_bTakeZen;
        int						m_nBackupTakeZen;
        int						m_nBackupInvenIndex;

    public:
        CNewUIStorageInventory();
        virtual ~CNewUIStorageInventory();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 2.2f

        CNewUIInventoryCtrl* GetInventoryCtrl() const;

        bool ProcessClosing();
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        int FindEmptySlot(ITEM* pItemObj);

        bool IsStorageLocked() { return m_bLock; }
        bool IsCorrectPassword() { return m_bCorrectPassword; }
        bool IsItemAutoMove() { return m_bItemAutoMove; }

        void SetBackupTakeZen(int nZen);

        bool ProcessMyInvenItemAutoMove();

        void SendRequestItemToMyInven(ITEM* pItemObj, int nStorageIndex, int nInvenIndex);

        void ProcessToReceiveStorageStatus(BYTE byStatus);
        void ProcessToReceiveStorageItems(int nIndex, std::span<const BYTE> pbyItemPacket);
        void ProcessStorageItemAutoMoveSuccess();
        void ProcessStorageItemAutoMoveFailure();

        int GetPointedItemIndex();

        void SetItemAutoMove(bool bItemAutoMove);
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
