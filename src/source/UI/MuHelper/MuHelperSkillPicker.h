#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <RmlUi/Core/Types.h>

#include <vector>

namespace Rml
{
    class ElementDocument;
}

namespace mu::ui::window
{
    class CManager;

    // One skill the player can assign, placed where native's fan-out put it.
    struct MuHelperSkillPickerEntry
    {
        float left = 0.f;       // reference px, in #panel's local space
        float top = 0.f;
        Rml::String decorator;  // "image(skill-icon-skill2-3-8)" -- see UI::Skills::IconSpriteName()
    };

    struct MuHelperSkillPickerRmlModel
    {
        // Docked-right window -- sourced from UI::Scaling::GetActiveTransform(), which CManager
        // scopes to LayoutMode::DockRight around every call into this window.
        float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        std::vector<MuHelperSkillPickerEntry> entries;
    };

    // The borderless flyout the MU Helper config window opens beside an attack or buff slot, listing
    // the skills that slot accepts. Picking one assigns it and closes the flyout.
    //
    // Input stays native on purpose: only the icons claim the mouse, and everything else passes
    // through -- the world, so the character keeps moving while the flyout is open (native blocked
    // that; changed deliberately), and the config window, which is how a second click on the same
    // slot reaches it and closes the flyout. Assignment happens on the native release, from the same
    // rects the icons are placed at, rather than through an RmlUi click whose ordering against
    // CManager's dispatch this frame would be a guess.
    class CMuHelperSkillPicker : public CObject
    {
    public:
        static constexpr float BoxWidth = 32.f;
        static constexpr float BoxHeight = 38.f;

        CMuHelperSkillPicker();
        ~CMuHelperSkillPicker() override;

        bool Create(CManager* pNewUIMng);
        void Release();

        bool UpdateMouseEvent() override;
        bool UpdateKeyEvent() override;
        bool Update() override;
        bool Render() override;
        float GetLayerDepth() override;

        void ReloadRmlTheme();

        void FilterByAttackSkills();
        void FilterByBuffSkills();

    private:
        struct Placement
        {
            int skillType = 0;
            float left = 0.f;
            float top = 0.f;
        };

        void PrepareSkillsToRender();
        void LayoutPlacements();
        int SkillUnderMouse() const;

        static bool IsAttackSkill(int iSkillType);
        static bool IsBuffSkill(int iSkillType);
        static bool IsHealingSkill(int iSkillType);
        static bool IsDefenseSkill(int iSkillType);

        void BuildRmlUi();
        void SyncRmlModel();

        CManager* m_pNewUIMng = nullptr;

        bool m_bFilterByAttackSkills = false;
        bool m_bFilterByBuffSkills = false;
        std::vector<int> m_aiSkillsToRender;
        std::vector<Placement> m_placements;
        bool m_bEntriesDirty = true;

        RmlModelBinder<MuHelperSkillPickerRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
    };
}
