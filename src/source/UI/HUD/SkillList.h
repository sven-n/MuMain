#pragma once

#include <vector>

#include "UI/Core/WindowObject.h"
#include "UI/Core/Window3DRenderMng.h"
#include "Render/Textures/ZzzTexture.h"
#include <RmlUi/Core/Types.h>

namespace mu::ui::window
{
    enum KINDOFSKILL
    {
        KOS_COMMAND = 1,
        KOS_SKILL1,
        KOS_SKILL2,
        KOS_SKILL3,
    };

    // One cell of the expanded skill grid or pet row, rebuilt every frame while open: position,
    // icon sprite, hotkey number, cooldown wipe and selection for main_frame.rml's .skill-cell.
    struct SkillCellEntry
    {
        float left = 0.f, top = 0.f;   // px, in #bars's local space: the cell's 32x38 box
        int skillIndex = -1;           // CharacterAttribute->Skill[] index (grid) or AT_PET_COMMAND_* value (pet row)
        bool isPet = false;            // true for pet-row entries -- click routes to the pet path
        bool isCurrent = false;        // Hero->CurrentSkill == skillIndex
        float cooldownFraction = 0.f;  // 0 = ready; shrinks toward 0 as the skill's delay counts down
        Rml::String icon = "none";     // decorator of the skill's icon (CSkillList::GetSkillIconDecorator())
        Rml::String hotkey;            // the skill's hotkey number, empty when it has none
    };

    class CSkillList : public CObject
    {
        enum
        {
            SKILLHOTKEY_COUNT = 10
        };

    public:
        enum IMAGE_LIST
        {
            IMAGE_SKILL1 = BITMAP_INTERFACE_NEW_SKILLICON_BEGIN,
            IMAGE_SKILL2,
            IMAGE_COMMAND,
            IMAGE_SKILL3,
            IMAGE_SKILLBOX,
            IMAGE_SKILLBOX_USE,
            IMAGE_NON_SKILL1,
            IMAGE_NON_SKILL2,
            IMAGE_NON_COMMAND,
            IMAGE_NON_SKILL3,
        };

        CSkillList();
        virtual ~CSkillList();

        bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        float GetLayerDepth();		// 10.6f

        WORD GetHeroPriorSkill();
        void SetHeroPriorSkill(BYTE bySkill);

        void Reset();

        void SetHotKey(int iHotKey, int iSkillType);
        int GetHotKey(int iHotKey);
        int GetSkillIndex(int iSkillType);

        // The icon decorator ("image(<sprite>)", or "none") for a skill/pet-command index, lit or
        // grey as the original's RenderSkillIcon() chose: UI::Skills::Icon::ResolveSkillIcon() on
        // this frame's usability.
        Rml::String GetSkillIconDecorator(int iIndex);
        Rml::String GetHotKeySlotIconDecorator(int iSlotIndex);
        Rml::String GetCurrentSkillIconDecorator();

        // The hotkey (0-9) a skill/pet-command index is bound to, or -1: the number the original
        // drew on every skill icon.
        int GetSkillHotKeyNumber(int iIndex);

        // Whether hotkey-row slot iSlotIndex (0-4) shows the active skill -- feeds
        // #skill_slot_0..4's selection highlight in SyncRmlModel().
        bool IsHotKeySlotCurrentSkill(int iSlotIndex);

        // The hotkey number (1-9,0) shown for slot iSlotIndex, for #skill_slot_0..4's label.
        // Returns -1 for an empty slot.
        int GetHotKeySlotNumber(int iSlotIndex);

        // Per-slot cooldown fraction for the compact hotkey row + current-skill slot (0 = ready).
        float GetHotKeySlotCooldownFraction(int iSlotIndex);
        float GetCurrentSkillCooldownFraction();

        // Naming trap: despite the name, this is whether the compact hotkey row shows its "upper"
        // set (6-9,0 vs 1-5), NOT whether the grid popup is open. Use IsSkillGridOpen() for that --
        // binding grid visibility to this method is the bug to avoid.
        bool IsSkillListUp();

        // The actual skill-grid-open state; see IsSkillListUp()'s comment for the similarly-named
        // method that is NOT this.
        bool IsSkillGridOpen() const { return m_bSkillList; }

        // Click/hover entry points bound from main_frame.rml's data-event-click/mouseover/mouseout.
        // Mouse-click behavior mirrors the legacy click branches, which never went through
        // UseHotKey() (so pet-check/auto-attack-cancel rules don't apply here either -- preserved
        // faithfully).
        void OnHotkeySlotClick(int iSlotIndex);
        // Positions: the hovered slot, icon or cell on screen, in the original's reference units
        // (scaled by the dp ratio), wherever the theme placed it.
        void OnHotkeySlotHover(int iSlotIndex, float slotLeft, float slotTop);
        void OnCurrentSkillClick();
        void OnCurrentSkillHover(float iconLeft, float iconTop);
        void OnGridCellClick(int iSkillIndex);
        void OnGridCellHover(int iSkillIndex, float cellLeft, float cellTop);
        void OnPetCellClick(int iSkillIndex);
        void OnPetCellHover(int iSkillIndex, float cellLeft, float cellTop);
        void OnUnhover();

        // Grid/pet overlay snapshots, rebuilt by Update() while the grid is open; copied into the
        // RmlUi model's skill_grid_cells/pet_skill_cells by SyncRmlModel().
        const std::vector<SkillCellEntry>& GetGridSnapshot() const { return m_GridSnapshot; }
        const std::vector<SkillCellEntry>& GetPetSnapshot() const { return m_PetSnapshot; }

        // True when a hover callback has queued a tooltip for SyncRmlModel(); cleared by
        // OnUnhover() or a new hover target.
        bool IsTooltipPending() const { return m_bTooltipPending; }
        int GetTooltipSkillIndex() const { return m_iTooltipSkillIndex; }
        float GetTooltipAnchorX() const { return m_fTooltipAnchorX; }
        float GetTooltipAnchorY() const { return m_fTooltipAnchorY; }

    private:
        void LoadImages();
        void UnloadImages();
        bool IsArrayUp(BYTE bySkill);
        bool IsArrayIn(BYTE bySkill);
        void UseHotKey(int iHotKey);

        // Rebuilds m_GridSnapshot/m_PetSnapshot from the grid/pet state each frame while open.
        void RebuildGridSnapshot();

        // Queues a tooltip for the given skill/pet-command index, anchored at (x, y) in #bars's
        // local space. Shared by all 5 hover entry points.
        void QueueTooltip(int iSkillIndex, float x, float y);

        void ResetMouseLButton();

    private:
        CManager* m_pNewUIMng;
        C3DRenderMng* m_pNewUI3DRenderMng;

        bool m_bHotKeySkillListUp;
        int m_iHotKeySkillType[SKILLHOTKEY_COUNT];

        bool m_bSkillList;

        WORD m_wHeroPriorSkill;

        std::vector<SkillCellEntry> m_GridSnapshot;
        std::vector<SkillCellEntry> m_PetSnapshot;

        bool m_bTooltipPending = false;
        int m_iTooltipSkillIndex = -1;
        float m_fTooltipAnchorX = 0.f, m_fTooltipAnchorY = 0.f;

        // Arms UpdateKeyEvent()'s Ctrl+digit SetHotKey() path; set by OnGridCellHover(), cleared
        // by OnUnhover(). -1 = nothing hovered.
        int m_iHoveredGridSkillIndex = -1;
    };
}
