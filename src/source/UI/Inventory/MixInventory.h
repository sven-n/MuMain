
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Widgets/UIControls.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include <span>
#include <vector>

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
        // Pre-layout fallback only -- WindowGeometry's real hit-box comes from #panel's own live
        // RCSS size (UI::RmlBridge::RefreshLogicalPanelSize(), read at the UpdateMouseEvent() call
        // site), seeded with these only for the first frame before RmlUi's layout has run. Never
        // referenced by the native mix-grid rendering, which has its own separate offset.
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

        // Window frame/title/Mix button/recipe-result content are all RmlUi now. Only the
        // inventory grid and the socket list box (CUISocketListBox, a real interactive widget, not
        // presentation) stay fully native -- same reasoning as CStorageInventoryExt for the grid.
        struct MixLine
        {
            Rml::String text;
            Rml::String color; // "rgba(r,g,b,a)"
            bool operator==(const MixLine&) const = default;
        };
        struct MixInventoryRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)
            Rml::String title;
            bool mixVisible = true;
            bool mixLocked = false;
            Rml::String mixTooltip;

            // Former RenderFrame()/RenderMixDescriptions() native text block, ported here --
            // see SyncMixContentModel()'s own comment for the byte-for-byte translation of each
            // field's source condition/color.
            bool showTaxRate = false;
            Rml::String taxRateText;

            bool showRecipe = false;
            Rml::String recipeLine1, recipeLine2;
            bool showRecipeLine2 = false;
            Rml::String recipeColor;

            bool showSuccessRate = false;
            Rml::String successRateText;
            Rml::String successRateColor;

            bool showRequiredZen = false;
            Rml::String requiredZenText;

            bool showPrediction = false;
            Rml::String predictionText;

            std::vector<MixLine> sourceLines;
            std::vector<MixLine> statusLines;
            std::vector<MixLine> adviceLines;
            std::vector<MixLine> descriptionLines;

            bool showSocketPrompt = false;
            Rml::String socketPromptText;
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

        void BuildRmlUi();
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

        void ReloadRmlTheme();

        float GetLayerDepth();	//. 3.4f

        CInventoryCtrl* GetInventoryCtrl() const;

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

    private:
        void LoadImages();
        void UnloadImages();

        bool InventoryProcess();
        bool BtnProcess();

        bool AutoMoveItem(CInventoryCtrl* srcCtrl, STORAGE_TYPE srcType,
            CInventoryCtrl* dstCtrl, STORAGE_TYPE dstType, bool requireMixSource);

        // Former RenderFrame()/RenderMixDescriptions() native text -- see its own comment
        // (MixInventory.cpp) for the full per-field translation.
        void SyncMixContentModel();

        void CheckMixInventory();
        bool Mix();
        void RenderMixEffect();

        int Rtn_MixRequireZen(int _nMixZen, int _nTax);
    };
}
