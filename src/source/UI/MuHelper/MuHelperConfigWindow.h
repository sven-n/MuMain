#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/MuHelper/MuHelperShared.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <RmlUi/Core/Types.h>

#include <array>
#include <vector>

namespace Rml
{
    class ElementDocument;
}

namespace mu::ui::window
{
    class CManager;

    // The class-specific controls a character sees -- UI::MuHelper::ResolveClassFeatures(), bound.
    struct MuHelperConfigFeatures
    {
        bool skill3 = false;
        bool combo = false;
        bool pet = false;
        bool party = false;
        bool autoHeal = false;
        bool drainLife = false;
        bool potionSummoner = false;

        bool operator==(const MuHelperConfigFeatures&) const = default;
    };

    // Every checkbox's state, read live from the staged config (they write it the moment they are
    // clicked, so it is the one source of truth).
    struct MuHelperConfigChecks
    {
        bool potion = false, longDistance = false, origPosition = false;
        bool skill2Delay = false, skill2Condition = false, skill3Delay = false, skill3Condition = false;
        bool combo = false, fallback = false, buffDuration = false;
        bool usePet = false, party = false, autoHeal = false, drainLife = false;
        bool repair = false, pickAll = false, pickSelected = false;
        bool pickJewel = false, pickAncient = false, pickZen = false, pickExcellent = false, pickExtra = false;
        bool autoFriend = false, autoGuild = false, autoDefend = false;

        bool operator==(const MuHelperConfigChecks&) const = default;
    };

    // The six skill slots' icon decorators ("image(skill-icon-...)" or "none").
    struct MuHelperSlotIcons
    {
        Rml::String s0, s1, s2, s3, s4, s5;

        bool operator==(const MuHelperSlotIcons&) const = default;
    };

    struct MuHelperExtraItem
    {
        Rml::String name;
        bool selected = false;
        int index = 0;
    };

    // Static strings, set once when the model is created.
    struct MuHelperConfigLabels
    {
        Rml::String title, tabHunting, tabObtaining, tabOther;
        Rml::String range, distance, basicSkill, activationSkill1, activationSkill2, seconds;
        Rml::String extensionUsed, extensionNone;
        Rml::String potion, longDistance, origPosition, delay, condition, combo, fallback, buffDuration;
        Rml::String usePet, party, autoHeal, drainLife, ravenCease, ravenAuto, ravenTogether;
        Rml::String repair, pickAll, pickSelected, pickJewel, pickAncient, pickZen, pickExcellent, pickExtra;
        Rml::String autoFriend, autoGuild, autoDefend;
        Rml::String setting, add, remove, save, init, closeTip;
    };

    struct MuHelperConfigRmlModel
    {
        float textPx = 0.f; // native text size in physical px (RmlNativeTextSize.h)

        int activeTab = 0;
        int huntRange = 0;
        int pickRange = 0;
        int ravenMode = 0;

        MuHelperConfigFeatures features;
        MuHelperConfigChecks checks;
        MuHelperSlotIcons slots;

        Rml::String distanceTime;
        Rml::String skill2Delay;
        Rml::String skill3Delay;
        Rml::String itemName;
        std::vector<MuHelperExtraItem> extraItems;

        MuHelperConfigLabels labels;
    };

    // The MU Helper's configuration window (the bot's settings), docked in the first column: three
    // tabs -- hunting, item pick-up, other -- over one staged copy of the settings
    // (UI::MuHelper::StagedConfig()), pushed to the bot and the server by Save.
    //
    // Commits the way native did. Checkboxes, the two range steppers and the extra-item list write
    // the staged config immediately; the six skill slots and the three number fields are held here
    // and written by Save (a slot also writes through when a skill is first assigned to it).
    class CMuHelperConfigWindow : public CObject
    {
    public:
        static constexpr int WindowWidth = 190;
        static constexpr int WindowHeight = 429;
        static constexpr int SkillSlotCount = 6;

        CMuHelperConfigWindow();
        ~CMuHelperConfigWindow() override;

        bool Create(CManager* pNewUIMng);
        Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
        void Release();

        bool UpdateMouseEvent() override;
        bool UpdateKeyEvent() override;
        bool Update() override;
        // So Escape still closes the window while one of its fields has the keyboard.
        bool TakesTypingFrom(const Rml::ElementDocument* document) const override
        {
            return document == m_RmlView.Document();
        }
        bool Render() override;
        float GetLayerDepth() override;
        float GetKeyEventOrder() override;
        void Show(bool bShow) override;

        // The settings the server sent, or the defaults (MainScene, on entering the game).
        void LoadSavedConfig(const MUHelper::ConfigData& config);
        void Reset();

        // The skill picker's choice for the slot that opened it.
        void AssignSkill(int iSkill);

    private:
        void ApplyConfig();
        void InitConfig();
        void SaveConfig();

        void OnToggle(const Rml::String& key);
        void OnButton(const Rml::String& key);
        void OnSlotClick(int slot);
        void OnSlotClear(int slot);
        void SelectTab(int tab);
        void AddExtraItem();
        void RemoveExtraItem();

        bool IsSkillAssigned(int iSkill) const;
        int GetSkillIndex(int iSkill) const;
        void ClearCombo();

        void BuildRmlUi();
        void SyncRmlModel();
        void RebuildExtraItems();
        void BlurFocusedField();

        CManager* m_pNewUIMng = nullptr;

        int m_iCurrentOpenTab = 0;
        int m_iSelectedSkillSlot = 0;
        std::array<int, SkillSlotCount> m_aiSelectedSkills{};
        Rml::String m_selectedExtraItem;
        bool m_bExtraItemsDirty = true;

        void BindRmlModel(Rml::DataModelConstructor& c, MuHelperConfigRmlModel& model);
        // After every build: the field filters and limits the theme cannot set.
        void OnRmlBuilt();

        UI::RmlBridge::ThemedView<MuHelperConfigRmlModel> m_RmlView{"mu_helper_config",
            [this](Rml::DataModelConstructor& c, MuHelperConfigRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/mu_helper_config.rml"}},
            {.afterBuild = [this] { OnRmlBuilt(); }}};
    };
}
