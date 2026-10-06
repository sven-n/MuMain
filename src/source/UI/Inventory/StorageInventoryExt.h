//*****************************************************************************
// File: NewUIStorageInventory.h
//*****************************************************************************

#pragma once

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
        // Pre-layout fallback only -- WindowGeometry's real hit-box comes from #panel's own live
        // RCSS size (UI::RmlBridge::RefreshLogicalPanelSize(), read at the UpdateMouseEvent() call
        // site), seeded with these only for the first frame before RmlUi's layout has run. Never
        // referenced by the native storage-grid rendering, which has its own separate offset.
        static constexpr float STORAGE_WIDTH = 190.0f;
        static constexpr float STORAGE_HEIGHT = 429.0f;

        CManager* m_pNewUIMng;
        POINT					m_Pos;

        CInventoryCtrl* m_pNewInventoryCtrl;

        bool					m_bItemAutoMove;
        int						m_nBackupMouseX;
        int						m_nBackupMouseY;
        int						m_nBackupSourceInvenIndex;

        // Window frame/title/exit button are RmlUi; the inventory grid stays native since its
        // icons are live 3D model renders (same reasoning as CMyInventory).
        struct StorageExtRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            float panelWidth = 190.f;
            float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)
            Rml::String title;
            Rml::String exitTooltip;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, StorageExtRmlModel& model);
        UI::RmlBridge::ThemedView<StorageExtRmlModel> m_RmlView{"storage_ext",
            [this](Rml::DataModelConstructor& c, StorageExtRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/storage_ext.rml"}}};

        // The frame background panel must render behind the grid's live 3D icons, but RmlUi's
        // main context always renders last -- so it goes through
        // RmlUiRuntime::GetBackgroundContext()/RenderBackgroundLayer() instead (see
        // CMyInventory's identical MyInventoryBgRmlModel for the full mechanism).
        struct StorageExtBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        static void BindRmlBgModel(Rml::DataModelConstructor& c, StorageExtBgRmlModel& model);
        UI::RmlBridge::ThemedView<StorageExtBgRmlModel> m_RmlBgView{"storage_ext_bg", BindRmlBgModel,
            {{"Data/Interface/RmlUi/storage_ext_bg.rml", [] { return RmlUiRuntime::Instance().GetBackgroundContext(); }}}};

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
        bool ProcessBtns() const;
        void ProcessStorageItemAutoMove();
    };
}
