
#pragma once

#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/Inventory/ItemGridModel.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Inventory/SocketListSelection.h"
#include "UI/RmlBridge/RmlThemedView.h"
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

        UI::Inventory::SocketListSelection m_SocketSelection;
        std::vector<DWORD> m_SocketItemKeys;
        bool m_SocketTextDirty = true;

        struct SocketListLine
        {
            Rml::String text;
            int index = 0;
            bool selected = false;
            bool operator==(const SocketListLine&) const = default;
        };
        struct MixLine
        {
            Rml::String text;
            // What the line is telling the player, which each theme colours: "missing" (a source
            // this recipe has none of), "partial", "ready", or for a description "normal",
            // "warning", "loss" (the item is destroyed on failure) and "dim".
            Rml::String kind;
            // Which row of the description block the line occupies -- RenderMixDescriptions()
            // skipped rows between some lines, so this is the row it chose, not its position in
            // the list. The theme turns it into a top. Unused (0/false) for the other line lists.
            int row = 0;
            bool alignLeft = false;
            // Native RenderText() shrinks a line to its box (FontScaleForBounds); this is that
            // factor for description lines, against a box kept inside the window (measured in the
            // window's own RmlUi font). 1 = fits.
            float fit = 1.f;
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
            float taxRateFit = 1.f; // MixLine::fit for the tax line (native 160 box)

            bool showRecipe = false;
            Rml::String recipeLine1, recipeLine2;
            bool showRecipeLine2 = false;
            bool recipeReady = false; // IsReadyToMix(); the theme colours the recipe name

            bool showSuccessRate = false;
            Rml::String successRateText;
            bool successBoosted = false; // a chaos-rate bonus is in the shown rate

            bool showRequiredZen = false;
            Rml::String requiredZenText;

            bool showPrediction = false;
            Rml::String predictionText;

            std::vector<MixLine> sourceLines;
            std::vector<MixLine> statusLines;
            std::vector<MixLine> adviceLines;
            std::vector<MixLine> descriptionLines;
        // The castle senior recipe's block starts a row lower than every other mix's.
        bool descriptionsLowered = false;

            bool showSocketPrompt = false;
            Rml::String socketPromptText;

            bool showSocketList = false;
            std::vector<SocketListLine> socketLines;
            // The grids as their documents draw them (CInventoryCtrl::Cells()).
            UI::Items::ItemGridCells gridCells;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, MixInventoryRmlModel& model);
        void OnRmlReloaded();
        UI::RmlBridge::ThemedView<MixInventoryRmlModel> m_RmlView{"mix_inventory",
            [this](Rml::DataModelConstructor& c, MixInventoryRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/mix_inventory.rml"}},
            {.afterReload = [this] { OnRmlReloaded(); }}};

        // The grids' items, into the document's #item_view.
        void RenderItems();
        UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItems(); },
                                                 this};

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

        float GetLayerDepth();	//. 3.4f

        CInventoryCtrl* GetInventoryCtrl() const;


    private:

        bool InventoryProcess();
        bool BtnProcess();

        bool AutoMoveItem(CInventoryCtrl* srcCtrl, STORAGE_TYPE srcType,
            CInventoryCtrl* dstCtrl, STORAGE_TYPE dstType, bool requireMixSource);

        // Former RenderFrame()/RenderMixDescriptions() native text -- see its own comment
        // (MixInventory.cpp) for the full per-field translation.
        void SyncMixContentModel();
        void SyncSocketListModel();
        bool RefreshSocketOptions();
        void SelectSocket(int index);
    bool PrepareSocketMix();
    void ConfirmMix(int mixType, int mixId, int socketIndex, const std::vector<DWORD>& itemKeys);

        bool CheckMixInventory();
        bool Mix();
        void RenderMixEffect();

        int Rtn_MixRequireZen(int _nMixZen, int _nTax);
    };
}
