
#if !defined(AFX_NEWUIMYSHOPINVENTORY_H__A0C3DD4A_C4D5_4CF2_9702_DF54540DB6FD__INCLUDED_)
#define AFX_NEWUIMYSHOPINVENTORY_H__A0C3DD4A_C4D5_4CF2_9702_DF54540DB6FD__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Widgets/UIControls.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CMyShopInventory : public CObject
    {
    public:
        enum SHOPTYEP
        {
            PERSONALSHOPSALE = 0,
            PERSONALSHOPPURCHASE,
        };

    private:
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        POINT m_Pos;

        // Window frame/title/edit-box background strip/Open-Close-Exit buttons are RmlUi; the
        // inventory grid stays native since its icons are live 3D model renders (same reasoning as
        // CMyInventory/CStorageInventoryExt). The nickname/subject CUITextInputBox (m_EditBox below)
        // also stays fully native -- it has no RmlUi equivalent yet.
        struct MyShopRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            Rml::String title;

            Rml::String exitTooltip;

            bool openLocked = false;
            Rml::String openTooltip;

            bool closeLocked = true;
            Rml::String closeTooltip;
        };
        RmlModelBinder<MyShopRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // The frame background panel must render behind the grid's live 3D icons, but RmlUi's main
        // context always renders last -- so it goes through
        // RmlUiRuntime::GetBackgroundContext()/RenderBackgroundLayer() instead (see
        // CStorageInventoryExt's identical StorageExtBgRmlModel for the full mechanism).
        struct MyShopBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        RmlModelBinder<MyShopBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

        // Mirrors the old CButton array's Lock()/tooltip-text state (OpenButtonLock()/UnLock(),
        // ChangePersonal()) now that the buttons themselves are RmlUi-owned.
        bool m_bOpenLocked;
        bool m_bOpenApplyTooltip; // true => "Apply" tooltip, false => "Open" tooltip

        void BuildRmlUi();
        void SyncRmlModel();

    public:
        CMyShopInventory();
        virtual ~CMyShopInventory();
        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();
        void SetPos(int x, int y);
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void ReloadRmlTheme();
        void ClosingProcess();
        float GetLayerDepth();	//. 3.2f

        CInventoryCtrl* GetInventoryCtrl() const;

    public:
        void ChangeSourceIndex(int sindex);
        const int GetSourceIndex();
        void ChangeTargetIndex(int tindex);
        const int GetTargetIndex();
        ITEM* FindItem(int iLinealPos);
        int GetItemInventoryIndex(ITEM* pItem);
        void ChangeEditBox(const UISTATES type);
        void ChangeTitle(wchar_t* titletext);
        void GetTitle(wchar_t* titletext);
        void SetTitle(wchar_t* titletext);
        void ChangePersonal(bool state);
        const bool IsEnablePersonalShop() const;
        void OpenButtonLock();
        void OpenButtonUnLock();
        int GetPointedItemIndex();
        void ResetSubject();
        bool IsEnableInputValueTextBox();
        void SetInputValueTextBox(bool bIsEnable);

    public:
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        void DeleteItem(int iIndex);
        void DeleteAllItems();

    private:
        bool MyShopInventoryProcess();
        bool WindowProcess();

    private:
        void RenderTextInfo();

    private:
        int					m_TargetIndex;
        int					m_SourceIndex;
        bool				m_EnablePersonalShop;
        bool				m_bIsEnableInputValueTextBox;

        CUITextInputBox* m_EditBox;
    };

    inline
        void CMyShopInventory::ChangeTitle(wchar_t* titletext)
    {
        m_EditBox->SetText(titletext);
    }

    inline
        void CMyShopInventory::ChangeSourceIndex(int sindex)
    {
        m_SourceIndex = sindex;
    }

    inline
        void CMyShopInventory::ChangeTargetIndex(int tindex)
    {
        m_TargetIndex = tindex;
    }

    inline
        float CMyShopInventory::GetLayerDepth()
    {
        return 3.2f;
    }

    inline
        CInventoryCtrl* CMyShopInventory::GetInventoryCtrl() const
    {
        return m_pNewInventoryCtrl;
    }

    inline
        const int CMyShopInventory::GetSourceIndex()
    {
        return m_SourceIndex;
    }

    inline
        const int CMyShopInventory::GetTargetIndex()
    {
        return m_TargetIndex;
    }

    // Shows the "enter selling price" GenericConfirmDialog. Factored out as a free function since
    // 3 of its 4 call sites already live in this file and 1 lives in ZzzInventory.cpp. Caller must
    // already have ChangeSourceIndex()/ChangeTargetIndex() set for the item being priced.
    void ShowPersonalShopItemValueDialog();
}

#endif // !defined(AFX_NEWUIMYSHOPINVENTORY_H__A0C3DD4A_C4D5_4CF2_9702_DF54540DB6FD__INCLUDED_)
