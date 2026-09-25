//*****************************************************************************
// File: NewUIStorageInventory.h
//*****************************************************************************

#if !defined(AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_)
#define AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CStorageInventory : public CObject
    {
    private:
        // Pre-layout fallback only -- WindowGeometry's real hit-box comes from #panel's own live
        // RCSS size (UI::RmlBridge::RefreshLogicalPanelSize(), read at the UpdateMouseEvent() call
        // site), seeded with these only for the first frame before RmlUi's layout has run. Never
        // referenced by the native storage-grid rendering, which has its own separate offset.
        static constexpr float STORAGE_WIDTH = 190.0f;
        static constexpr float STORAGE_HEIGHT = 429.0f;

        CManager* m_pNewUIMng;
        POINT					m_Pos;
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

        // Window frame/title/money/buttons are RmlUi; the inventory grid stays native since its
        // icons are live 3D model renders (same reasoning as CMyInventory/CStorageInventoryExt).
        struct StorageRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;

            Rml::String title;
            bool titleLocked = false; // legacy-only red/gray title color toggle -- see SyncRmlModel()

            Rml::String zenText;
            Rml::String zenTier; // UI::RmlBridge::GoldTierKey() of the amount, legacy only
            Rml::String feeLabel;
            Rml::String feeValue;

            bool expandVisible = false;
            Rml::String expandTooltip;

            bool storageLocked = false; // lock icon open/closed toggle, mirrors my_inventory's close-mode

            Rml::String insertTooltip;
            Rml::String takeTooltip;
            Rml::String lockTooltip;
        };
        RmlModelBinder<StorageRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // The frame background panel must render behind the grid's live 3D icons, but RmlUi's
        // main context always renders last -- so it goes through
        // RmlUiRuntime::GetBackgroundContext()/RenderBackgroundLayer() instead (see
        // CMyInventory's identical MyInventoryBgRmlModel for the full mechanism).
        struct StorageBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        RmlModelBinder<StorageBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

        void BuildRmlUi();
        void SyncRmlModel();

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

        void ReloadRmlTheme();

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
