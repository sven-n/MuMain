//*****************************************************************************
// File: NewUIStorageInventory.h
//*****************************************************************************

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include <span>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CStorageInventoryExt : public CObject
    {
    private:
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
            Rml::String title;
            Rml::String exitTooltip;
        };
        RmlModelBinder<StorageExtRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // The frame background panel must render behind the grid's live 3D icons, but RmlUi's
        // main context always renders last -- so it goes through
        // RmlUiRuntime::GetBackgroundContext()/RenderBackgroundLayer() instead (see
        // CMyInventory's identical MyInventoryBgRmlModel for the full mechanism).
        struct StorageExtBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        RmlModelBinder<StorageExtBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

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
