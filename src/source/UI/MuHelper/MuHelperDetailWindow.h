#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <RmlUi/Core/Types.h>

namespace Rml
{
    class ElementDocument;
    class Event;
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
        float textPx = 0.f; // native text size in physical px (RmlNativeTextSize.h)

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

        bool Create(CManager* pNewUIMng);
        Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
        void Release();

        bool UpdateMouseEvent() override;
        bool UpdateKeyEvent() override;
        bool Update() override;
        // Native's text box swallowed Escape; claimed deliberately so Escape closes the panel from
        // inside the buff-interval field too.
        bool TakesTypingFrom(const Rml::ElementDocument* document) const override
        {
            return document == m_RmlView.Document();
        }
        bool Render() override;
        float GetLayerDepth() override;
        float GetKeyEventOrder() override;
        void Show(bool bShow) override;

        // Opens `iPage`, or closes the panel if it is already showing that page.
        void Toggle(int iPage);
        void Save();
        void Reset();
        void ApplySavedConfig();
        void InitConfig();

    private:
        // RmlUi owns each gauge's hit area; C++ keeps the 0..10 threshold behavior.
        void HandleGaugeEvent(Rml::Event& event, int gauge);

        void SetPreCondition(int index);
        void SetSubCondition(int index);
        void RehydrateConditions();

        void BuildRmlUi();
        void SyncRmlModel();
        void BlurFocusedField();

        CManager* m_pNewUIMng = nullptr;

        int m_iCurrentPage = -1;
        int m_iCurrentPotionThreshold = 0;
        int m_iCurrentHealThreshold = 0;
        int m_iCurrentPartyHealThreshold = 0;

        void BindRmlModel(Rml::DataModelConstructor& c, MuHelperDetailRmlModel& model);
        // After every build: the field filters and limits the theme cannot set.
        void OnRmlBuilt();

        UI::RmlBridge::ThemedView<MuHelperDetailRmlModel> m_RmlView{"mu_helper_detail",
            [this](Rml::DataModelConstructor& c, MuHelperDetailRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/mu_helper_detail.rml"}},
            {.afterBuild = [this] { OnRmlBuilt(); }}};
    };
}
