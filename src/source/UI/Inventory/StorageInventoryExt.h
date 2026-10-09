//*****************************************************************************
// File: NewUIStorageInventory.h
//*****************************************************************************

#pragma once

#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/Inventory/ItemGridModel.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include <span>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CStorageInventoryExt : public CObject
    {
    private:

        CManager* m_pNewUIMng;
        POINT					m_Pos;

        CInventoryCtrl* m_pNewInventoryCtrl;

        bool					m_bItemAutoMove;
        // The storage and inventory cells under the pointer when an auto-move was asked.
        int m_nBackupStorageCell = -1;
        int m_nBackupInventoryCell = -1;
        int						m_nBackupSourceInvenIndex;

        // Window frame/title/exit button are RmlUi; the inventory grid stays native since its
        // icons are live 3D model renders (same reasoning as CMyInventory).
        struct StorageExtRmlModel
        {
            float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)
            Rml::String title;
            Rml::String exitTooltip;
            // The grids as their documents draw them (CInventoryCtrl::Cells()).
            UI::Items::ItemGridCells gridCells;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, StorageExtRmlModel& model);
        UI::RmlBridge::ThemedView<StorageExtRmlModel> m_RmlView{"storage_ext",
            [this](Rml::DataModelConstructor& c, StorageExtRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/storage_ext.rml"}}};

        // The grids' items, into the document's #item_view.
        void RenderItems();
        UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItems(); }};

        void BuildRmlUi();
        void SyncRmlModel();

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
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
        // The pointer over the drawn panel.
        bool IsPointerOverPanel();

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
        void DeleteAllItems() const;

        void ProcessInventoryCtrl();
        void ProcessStorageItemAutoMove();
    };
}
