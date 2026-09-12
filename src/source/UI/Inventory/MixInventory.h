
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Widgets/UIControls.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include <span>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CMixInventory : public CObject
    {
    public:
        enum MIX_STATE
        {
            MIX_READY = 0,
            MIX_REQUESTED,
            MIX_FINISHED
        };

    private:
        static constexpr float INVENTORY_WIDTH = 190.0f;
        static constexpr float INVENTORY_HEIGHT = 429.0f;

        CManager* m_pNewUIMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        POINT m_Pos;

        int m_iMixState;
        int m_iMixEffectTimer;
        float m_fInventoryColor[3];
        float m_fInventoryWarningColor[3];

        CUISocketListBox m_SocketListBox;

        // Window frame/title/Mix button are RmlUi. The inventory grid, the socket list box, and
        // the large native recipe/tax-rate/success-rate description panel (RenderMixDescriptions()
        // and the dynamic text block RenderFrame() still draws directly) stay fully native and
        // untouched -- same reasoning as CStorageInventoryExt for the grid, plus this window's own
        // out-of-scope text panel.
        struct MixInventoryRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            Rml::String title;
            bool mixVisible = true;
            bool mixLocked = false;
            Rml::String mixTooltip;
        };
        RmlModelBinder<MixInventoryRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // The frame background panel must render behind the grid's live 3D icons, but RmlUi's
        // main context always renders last -- so it goes through
        // RmlUiRuntime::GetBackgroundContext()/RenderBackgroundLayer() instead (see
        // CMyInventory's identical MyInventoryBgRmlModel for the full mechanism).
        struct MixInventoryBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        RmlModelBinder<MixInventoryBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

        void SyncRmlModel();

    public:
        CMixInventory();
        virtual ~CMixInventory();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        bool ProcessMyInvenItemAutoMove(CInventoryCtrl* sourceCtrl = nullptr);
        bool ProcessMixItemAutoMoveToInventory();
        void DeleteItem(int iIndex);
        void DeleteAllItems();

        void OpeningProcess();
        bool ClosingProcess();

        void SetMixState(int iMixState);
        int GetMixState() { return m_iMixState; }

        int GetPointedItemIndex();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 3.4f

        CInventoryCtrl* GetInventoryCtrl() const;

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        bool InventoryProcess();
        bool BtnProcess();

        bool AutoMoveItem(CInventoryCtrl* srcCtrl, STORAGE_TYPE srcType,
            CInventoryCtrl* dstCtrl, STORAGE_TYPE dstType, bool requireMixSource);

        void RenderMixDescriptions(float fPos_x, float fPos_y);

        void CheckMixInventory();
        bool Mix();
        void RenderMixEffect();

        int Rtn_MixRequireZen(int _nMixZen, int _nTax);
    };
}
