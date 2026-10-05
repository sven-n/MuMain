#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <RmlUi/Core/Types.h>

namespace Rml
{
    class ElementDocument;
}

namespace mu::ui::window
{
    class CManager;

    // Static strings, set once when the model is created.
    struct MuHelperDetailLabels
    {
        Rml::String titleActivation, titleRecovery, titleParty;
        Rml::String panePreCon, paneSubCon, paneAutoPotion, paneAutoHeal, paneDrainLife;
        Rml::String paneBuffSupport, paneHealSupport;
        Rml::String hpStatus, hpStatusParty;
        Rml::String preconHuntRange, preconAttacking;
        Rml::String subconTwo, subconThree, subconFour, subconFive;
        Rml::String partyHeal, partyDuration, timeSpace;
        Rml::String save, init, closeTip;
    };

    struct MuHelperDetailRmlModel
    {
        // Docked-right window -- sourced from UI::Scaling::GetActiveTransform(), which CManager
        // scopes to LayoutMode::DockRight around every call into this window.
        float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

        int page = -1;       // EMuHelperDetailPage
        int precon = -1;     // 0 = monster within hunting range, 1 = monster attacking me
        int subcon = -1;     // 0..3 = more than 2..5 monsters

        // 0..10, the gauges' fill in tenths.
        int potionLevel = 0;
        int healLevel = 0;
        int partyHealLevel = 0;

        bool partyHeal = false;
        bool partyDuration = false;
        Rml::String buffInterval;

        MuHelperDetailLabels labels;
    };

    // The MU Helper's detail panel, docked beside the config window: one page per "Setting" button
    // -- a skill's activation condition, the potion / heal / drain-life thresholds, or the party
    // support options.
    //
    // Two kinds of edit, committed differently, exactly as native did: the condition radios and
    // the two party checkboxes write the staged config the moment they are clicked, while the
    // three threshold gauges and the buff interval are held here and only committed by Save.
    // Closing without saving therefore keeps the first kind and discards the second.
    class CMuHelperDetailWindow : public CObject
    {
    public:
        static constexpr int WindowWidth = 190;
        static constexpr int WindowHeight = 429;

        CMuHelperDetailWindow();
        ~CMuHelperDetailWindow() override;

        bool Create(CManager* pNewUIMng, int x, int y);
        Rml::ElementDocument* GetFillDocument() const override { return m_pRmlDoc; }
        void SetPos(int x, int y) { m_Pos = {x, y}; }
        void Release();

        bool UpdateMouseEvent() override;
        bool UpdateKeyEvent() override;
        bool Update() override;
        bool Render() override;
        float GetLayerDepth() override;
        float GetKeyEventOrder() override;
        void Show(bool bShow) override;

        void ReloadRmlTheme();

        // Opens `iPage`, or closes the panel if it is already showing that page.
        void Toggle(int iPage);
        void Save();
        void Reset();
        void ApplySavedConfig();
        void InitConfig();

    private:
        // Native's gauge input, kept in C++: a click maps to floor(10 * x / width) + 1, the wheel
        // steps by one, and holding the button drags. A stock <input type="range"> rounds
        // differently and has no wheel handling. The bar is wherever the theme draws `gaugeId`;
        // `left`/`top` (and a 124 width) are the original's place, used until it has laid out.
        bool UpdateGauge(const char* gaugeId, int left, int top, int& level, const POINT& panelPos);

        void SetPreCondition(int index);
        void SetSubCondition(int index);
        void RehydrateConditions();

        void BuildRmlUi();
        void SyncRmlModel();
        void BlurFocusedField();

        CManager* m_pNewUIMng = nullptr;
        POINT m_Pos{};

        int m_iCurrentPage = -1;
        int m_iCurrentPotionThreshold = 0;
        int m_iCurrentHealThreshold = 0;
        int m_iCurrentPartyHealThreshold = 0;

        RmlModelBinder<MuHelperDetailRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
    };
}
