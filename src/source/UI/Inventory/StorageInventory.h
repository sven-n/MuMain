//*****************************************************************************
// File: NewUIStorageInventory.h
//*****************************************************************************

#if !defined(AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_)
#define AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_

#pragma once

#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/Inventory/ItemGridModel.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Inventory/StorageUpdates.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CStorageInventory : public CObject
    {
    private:

        CManager* m_pNewUIMng;
        POINT					m_Pos;
        CInventoryCtrl* m_pNewInventoryCtrl;

        bool					m_bLock;
        bool					m_bCorrectPassword;

        bool					m_bItemAutoMove;
        // The storage and inventory cells under the pointer when an auto-move was asked.
        int m_nBackupStorageCell = -1;
        int m_nBackupInventoryCell = -1;

        bool					m_bTakeZen;
        int						m_nBackupTakeZen;
        int						m_nBackupInvenIndex;
        int						m_nBackupSourceInvenIndex;

        // Window frame/title/money/buttons are RmlUi; the inventory grid stays native since its
        // icons are live 3D model renders (same reasoning as CMyInventory/CStorageInventoryExt).
        struct StorageRmlModel
        {
            float textPx = 0.f; // native text size in physical px (RmlNativeTextSize.h)

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
            // The grids as their documents draw them (CInventoryCtrl::Cells()).
            UI::Items::ItemGridCells gridCells;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, StorageRmlModel& model);
        UI::RmlBridge::ThemedView<StorageRmlModel> m_RmlView{"storage",
            [this](Rml::DataModelConstructor& c, StorageRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/storage.rml"}}};

        // The grids' items, into the document's #item_view.
        void RenderItems();
        UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItems(); }};

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
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
        // The pointer over the drawn panel.
        bool IsPointerOverPanel();

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

        void ProcessToReceiveStorageStatus(UI::Storage::VaultStatus status);
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
        void ProcessStorageItemAutoMove();
    };
}

#endif // !defined(AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_)
