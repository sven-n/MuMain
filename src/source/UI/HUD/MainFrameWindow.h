
#if !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
#define AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_

#pragma once

#include <vector>

#include "UI/Core/WindowObject.h"
#include "Render/Textures/ZzzTexture.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/RmlBridge/RmlRenderTarget.h"

#include <memory>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    enum
    {
        HOTKEY_Q = 0,
        HOTKEY_W,
        HOTKEY_E,
        HOTKEY_R,
        HOTKEY_COUNT
    };

    enum
    {
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        MAINFRAME_BTN_PARTCHARGE = 0,
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
        MAINFRAME_BTN_CHAINFO,
        MAINFRAME_BTN_MYINVEN,
        MAINFRAME_BTN_FRIEND,
        MAINFRAME_BTN_WINDOW,
    };

    enum KINDOFSKILL
    {
        KOS_COMMAND = 1,
        KOS_SKILL1,
        KOS_SKILL2,
        KOS_SKILL3,
    };

    class CItemHotKey
    {
    public:
        CItemHotKey();
        virtual ~CItemHotKey();

        bool UpdateKeyEvent();

        void SetHotKey(int iHotKey, int iItemType, int iItemLevel);
        int GetHotKey(int iHotKey);
        int GetHotKeyLevel(int iHotKey);

        // Each icon is a render target its slot's .item-icon shows, so RCSS places it like the rest
        // of the slot. Once a frame from SyncRmlModel(): sizes each target to its icon's on-screen
        // box and points the icon at it.
        void SyncSlotIcons(Rml::ElementDocument* document);
        // From SyncDocVisibility(), which runs whether or not this window updates.
        void SetSlotIconsShown(bool shown);

        // RmlUi handles hit-testing/hover for the 4 item-hotkey slots; sets a member read by
        // SyncRmlModel() and by the icon's own drawer.
        void OnHotkeySlotHover(int iSlotIndex) { m_iHoveredSlot = iSlotIndex; }
        void OnUnhover() { m_iHoveredSlot = -1; }
        int GetHoveredSlot() const { return m_iHoveredSlot; }
        void OnHotkeySlotRightClick(int iSlotIndex);

        // Stack count for the slot's bound item (0 if unbound/non-stacking); feeds #item_slots' stack-label binding.
        int GetSlotItemCount(int iSlotIndex);

    private:
        int GetHotKeyItemIndex(int iType, bool bItemCount = false);
        bool GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd);
        int GetHotKeyItemCount(int iType);
        ITEM* GetSlotItem(int iSlotIndex);
        // A target's drawer: one frame of the slot's item, framed for `width` x `height`.
        void RenderSlot(int iSlotIndex, std::uint32_t width, std::uint32_t height);

        int m_iHotKeyItemType[HOTKEY_COUNT];
        int m_iHotKeyItemLevel[HOTKEY_COUNT];
        std::unique_ptr<UI::RmlBridge::RenderTarget> m_SlotTargets[HOTKEY_COUNT];
        bool m_bSlotIconsShown = false;

        // -1 = nothing hovered; set by OnHotkeySlotHover(), cleared by OnUnhover().
        int m_iHoveredSlot = -1;
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

    // This file welds three classes: this one (frame chrome, HP/MP/AG/SD/EXP bars, corner
    // buttons), CSkillList (hotkey row, grid, pet commands) and CItemHotKey (Q/W/E/R slots, whose
    // native 3D icons are drawn into render targets). All of it is drawn by main_frame.rml.
    //
    // UpdateMouseEvent() always reports "not consumed": RmlUi hit-tests the HUD. UpdateKeyEvent()
    // still handles the Q/W/E/R item hotkeys.
    //
    // Known simplifications: no "gained EXP" flash overlay; readouts are plain text instead of the
    // digit-sprite atlas.
    class CMainFrameWindow : public CObject
    {
    public:
        CMainFrameWindow();
        virtual ~CMainFrameWindow();

        bool Create(CManager* pNewUIMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool IsVisible() const;

        // Whether the cursor is on a part of the HUD that takes the pointer, wherever the theme
        // placed it -- so world clicks there don't go through.
        bool IsMouseOverHud() const;

        void ReloadRmlTheme();

        float GetLayerDepth();		// 10.2f
        float GetKeyEventOrder();	// 7.f

        void SetItemHotKey(int iHotKey, int iItemType, int iItemLevel);
        int GetItemHotKey(int iHotKey);
        int GetItemHotKeyLevel(int iHotKey);
        //void RenderHotKeyItems();
        void UpdateItemHotKey();

        void ResetSkillHotKey();
        void SetSkillHotKey(int iHotKey, int iSkillType);
        int GetSkillHotKey(int iHotKey);
        int GetSkillHotKeyIndex(int iSkillType);

        void SetPreExp_Wide(__int64 dwPreExp);
        void SetGetExp_Wide(__int64 dwGetExp);

        void SetPreExp(__int64 dwPreExp);
        void SetGetExp(__int64 dwGetExp);

        // Called externally when a button's target panel opens/closes, to sync its bound "open" model boolean.
        void SetBtnState(int iBtnType, bool bStateDown);

        // Set by the RmlUi document's data-event-click bindings; polled and cleared like other windows' RmlClickX().
        void RmlClickCShop() { m_bRmlCShopClicked = true; }
        void RmlClickChaInfo() { m_bRmlChaInfoClicked = true; }
        void RmlClickMyInven() { m_bRmlMyInvenClicked = true; }
        void RmlClickFriend() { m_bRmlFriendClicked = true; }
        void RmlClickWindow() { m_bRmlWindowClicked = true; }

    private:
        void SyncRmlModel();
        void BuildRmlUi();

    public:
        __int64	m_loPreExp;
        __int64	m_loGetExp;

    private:
        CManager* m_pNewUIMng;

        CItemHotKey m_ItemHotKey;

        bool m_bExpEffect;
        DWORD m_dwExpEffectTime;

        __int64 m_dwPreExp;
        __int64 m_dwGetExp;

        bool m_bButtonBlink;

        struct MainFrameRmlModel
        {
            // The original's gauge and button hint text size in real pixels: RenderTipText() under
            // the HUD's scale, which grows about half as fast as the bars. The HUD itself is sized
            // in dp by main_frame.rcss.
            float hintPx = 0.f;

            float hpFraction = 0.f, mpFraction = 0.f, agFraction = 0.f, sdFraction = 0.f;
            Rml::String hpText, mpText, agText, sdText;

            // Current-value-only readout ("935", not "935 / 935") -- legacy theme's gauge numbers
            // bind this instead of hpText/etc; both computed unconditionally, themes just
            // reference different fields.
            Rml::String hpCurrentText, mpCurrentText, agCurrentText, sdCurrentText;
            Rml::String hpTooltip, mpTooltip, agTooltip, sdTooltip;
            bool poisoned = false; // true -> HP fill swaps red to green (eDeBuff_Poison)

            // expFraction is progress within the current 10%-of-level decile (0..1), not overall
            // level progress; expDigit (0-9) is that decile number. Position comes from the shared
            // bars transform group, spanning x=0 to x=640 (not the full window width the legacy
            // band used).
            float expFraction = 0.f;
            Rml::String expDigit;
            Rml::String expTooltip;

            // Corner buttons: "Open" mirrors SetBtnState()'s bStateDown; tooltips are static strings pushed once.
            bool cShopOpen = false, chaInfoOpen = false, myInvenOpen = false, friendOpen = false, windowOpen = false;
            Rml::String cShopTooltip, chaInfoTooltip, myInvenTooltip, friendTooltip, windowTooltip;

            // Alert-blink dots: bound boolean + CSS dot, computed each frame from the same state
            // the legacy blink read.
            bool chaInfoAlert = false, friendAlert = false;

            // Which of #skill_slot_0..4 shows the active skill (see
            // CSkillList::IsHotKeySlotCurrentSkill()). 5 named fields, not an array -- RmlUi's
            // DataModelConstructor binds pointer-to-member scalars only.
            bool skillSlot0Selected = false, skillSlot1Selected = false, skillSlot2Selected = false,
                 skillSlot3Selected = false, skillSlot4Selected = false;

            // Hotkey number label per slot (see CSkillList::GetHotKeySlotNumber()); empty string
            // for an unbound slot renders nothing.
            Rml::String skillSlot0Hotkey, skillSlot1Hotkey, skillSlot2Hotkey, skillSlot3Hotkey, skillSlot4Hotkey;

            // Icon decorator per hotkey slot and for the current-skill slot (see
            // CSkillList::GetSkillIconDecorator()), and the current skill's hotkey number.
            Rml::String skillSlot0Icon = "none", skillSlot1Icon = "none", skillSlot2Icon = "none",
                        skillSlot3Icon = "none", skillSlot4Icon = "none";
            Rml::String currentSkillIcon = "none";
            Rml::String currentSkillHotkey;

            // Cooldown wipe for the compact row + current-skill slot; same per-field convention as skillSlot0..4Hotkey.
            float skillSlot0Cooldown = 0.f, skillSlot1Cooldown = 0.f, skillSlot2Cooldown = 0.f,
                  skillSlot3Cooldown = 0.f, skillSlot4Cooldown = 0.f;
            float currentSkillCooldown = 0.f;

            // Mirrors CSkillList::IsSkillGridOpen() (NOT the similarly-named IsSkillListUp() --
            // see that method's comment). Gates #skill_grid/#pet_skill_row visibility; cell arrays
            // are rebuilt each frame while open, left stale (harmless, hidden) while closed.
            bool skillGridOpen = false;
            // CSkillList::IsSkillListUp(): the compact row shows its upper set, which the theme
            // highlights (#skill_list_highlight).
            bool skillListUp = false;
            std::vector<SkillCellEntry> skillGridCells;
            std::vector<SkillCellEntry> petSkillCells;

            // Item hotkey chrome (#item_slots): hover border + stack-count for the 4 Q/W/E/R
            // slots. The 3D-rendered potion icon itself is untouched (permanent native content).
            bool itemSlot0Hovered = false, itemSlot1Hovered = false, itemSlot2Hovered = false, itemSlot3Hovered = false;
            Rml::String itemSlot0Count, itemSlot1Count, itemSlot2Count, itemSlot3Count;
        };
        RmlModelBinder<MainFrameRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        // A theme's top-right button row (main_frame_top.rml, modern only), bound to m_RmlBinder's
        // model: a document of its own so it stacks under the windows docked over that corner
        // while the bars stay over them (RmlStackingOrder.cpp).
        Rml::ElementDocument* m_pRmlTopDoc = nullptr;

        bool m_bRmlCShopClicked = false;
        bool m_bRmlChaInfoClicked = false;
        bool m_bRmlMyInvenClicked = false;
        bool m_bRmlFriendClicked = false;
        bool m_bRmlWindowClicked = false;

    public:
        // Gates the RmlUi doc's Show()/Hide() on IsVisible() && sceneAllowsShow, same as CMuHelperBar/CBuffStrip.
        void SyncDocVisibility(bool sceneAllowsShow);
    };
}

#endif // !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
