#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/NewUIButton.h"

namespace mu::ui::window
{
    class CInventoryExtension : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_NPCSHOP_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
            IMAGE_NPCSHOP_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP2,
            IMAGE_NPCSHOP_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_NPCSHOP_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_NPCSHOP_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_INVENTORY_EXIT_BTN = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_EXTENSION_EMPTY = BITMAP_INTERFACE_NEW_INVENTORY_EXT_BEGIN,
            IMAGE_EXTENSION_TABLE,
            IMAGE_EXTENSION_NO1,
            IMAGE_EXTENSION_NO2,
            IMAGE_EXTENSION_NO3,
            IMAGE_EXTENSION_NO4,
        };

    private:
        static constexpr float WIDTH = 190.0f;
        static constexpr float HEIGHT = 429.0f;
        static constexpr float HEIGHT_PER_EXT = 87.0f;
        static constexpr float EXT_BORDER = 3.0f;

        CManager* m_pNewUIMng;
        CInventoryCtrl* m_extensions[MAX_INVENTORY_EXT_COUNT];
        POINT m_Pos;

        CButton m_BtnExit;
    protected:
        void SetButtonInfo();
    public:
        CInventoryExtension();
        virtual ~CInventoryExtension();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 2.5f
        ITEM* FindItem(int iIndex) const;
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const;
        void DeleteItem(int iIndex) const;
        void DeleteAllItems() const;
        int FindEmptySlot(int cx, int cy, const CInventoryCtrl* excluded = nullptr) const;
        CInventoryCtrl* GetOwnerOf(const CPickedItem* pPickedItem) const;
    private:
        void Init();

        void LoadImages();
        void UnloadImages();
        CInventoryCtrl* TryGetExtensionByInventoryIndex(int iIndex) const;

        bool InventoryProcess();

        void RenderFrame() const;
        void RenderTexts() const;
    };
}
