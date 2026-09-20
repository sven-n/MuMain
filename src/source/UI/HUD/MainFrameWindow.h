
#if !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
#define AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_

#pragma once

#include <vector>

#include "UI/Core/WindowObject.h"
#include "Render/Textures/ZzzTexture.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/RmlBridge/RmlModelBinder.h"

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
        void RenderItems();

        // RmlUi now handles hit-testing/hover for the 4 item-hotkey slots (icon art stays native);
        // sets a member read by SyncRmlModel() next frame.
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

        int m_iHotKeyItemType[HOTKEY_COUNT];
        int m_iHotKeyItemLevel[HOTKEY_COUNT];

        // -1 = nothing hovered; set by OnHotkeySlotHover(), cleared by OnUnhover().
        int m_iHoveredSlot = -1;
    };

    // One interactive overlay cell in the expanded skill grid or pet row, rebuilt every frame
    // while open. Icon/box art is NOT here -- it stays a legacy 2D draw (RenderSkillIcon()'s atlas
    // addressing is too irregular to port blind); this struct only drives RmlUi's hit target,
    // cooldown wipe, and selection highlight.
    struct SkillCellEntry
    {
        float left = 0.f, top = 0.f;   // px, in #bars's local space; matches the legacy icon/box position
        int skillIndex = -1;           // CharacterAttribute->Skill[] index (grid) or AT_PET_COMMAND_* value (pet row)
        bool isPet = false;            // true for pet-row entries -- click routes to the pet path
        bool isCurrent = false;        // Hero->CurrentSkill == skillIndex
        float cooldownFraction = 0.f;  // 0 = ready; shrinks toward 0 as the skill's delay counts down
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
        void RenderCurrentSkillAndHotSkillList();

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
        void OnHotkeySlotHover(int iSlotIndex);
        void OnCurrentSkillClick();
        void OnCurrentSkillHover();
        void OnGridCellClick(int iSkillIndex);
        void OnGridCellHover(int iSkillIndex);
        void OnPetCellClick(int iSkillIndex);
        void OnPetCellHover(int iSkillIndex);
        void OnUnhover();

        // Grid/pet overlay snapshots, rebuilt by Update() while the grid is open; copied into the
        // RmlUi model's skill_grid_cells/pet_skill_cells by SyncRmlModel() and also read by
        // Render()'s legacy icon draw.
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

        void RenderSkillIcon(int iIndex, float x, float y, float width, float height);
        // Cooldown math now lives in ComputeSkillCooldownFraction() (MainFrameWindow.cpp);
        // pet-row drawing is inlined into Render().

        // Rebuilds m_GridSnapshot/m_PetSnapshot from the grid/pet state each frame while open;
        // Render() iterates the result instead of recomputing positions.
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

    // This file welds three classes: this one (frame chrome + HP/MP/AG/SD/EXP bars + 5 corner
    // buttons, RmlUi), CSkillList (hotkey row/grid/pet commands, still legacy), and CItemHotKey
    // (QWER item slots, permanently native 3D icon render -- only #item_slots' hover/stack-count
    // chrome moved to RmlUi).
    //
    // Render() is a thin passthrough, not a full no-op: RmlUi always paints last in the frame, so
    // moving the center-band background chrome to RmlUi would occlude the still-legacy skill row
    // painted earlier. Render() still calls RenderLeftFrame()/RenderCenterFrame() (chrome for
    // regions with legacy content) and RenderCurrentSkillAndHotSkillList(); the chrome/parts for
    // fully-RmlUi regions (right frame, exp background, buttons, gauges) are gone.
    //
    // UpdateMouseEvent() drops BtnProcess() (RmlUi now hit-tests the corner buttons) and always
    // reports "not consumed". UpdateKeyEvent() is unchanged -- still gates legacy CItemHotKey
    // Q/W/E/R handling.
    //
    // Known simplifications: no "gained EXP" flash overlay; HP/MP/AG/SD/EXP readouts use plain
    // RCSS text instead of a digit-sprite atlas; gauge fill is a flat color, not the legacy
    // texture (a stretched `image()` decorator would visibly distort it, and clipped-oversized-
    // image doesn't work in this RmlUi build -- see CBuffStrip); corner buttons only reproduce
    // "normal"/"panel-open" frames, with hover done via a CSS brightness filter.
    class CMainFrameWindow : public CObject, public I3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            // Gauge/button textures are loaded by RmlUi directly (see main_frame.rcss's
            // @spritesheet blocks), not here.
            IMAGE_MENU_1 = BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN,	// newui_menu01.jpg
            IMAGE_MENU_2,		// newui_menu02.jpg
            IMAGE_MENU_3,		// newui_menu03.jpg
            IMAGE_MENU_2_1,
        };

        CMainFrameWindow();
        virtual ~CMainFrameWindow();

        bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void Render3D();

        bool IsVisible() const;

        void ReloadRmlTheme() override;

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

        // Pixel X offset read from main_frame.rml's #item_hotkey_anchor/#skill_list_anchor
        // markers, so the still-legacy item-hotkey/skill-hotkey bands follow wherever the active
        // theme's RCSS positions them. CSkillList reads GetSkillListOffsetX() via g_pMainFrame.
        float GetItemHotkeyOffsetX() const { return m_fItemHotkeyOffsetX; }
        float GetSkillListOffsetX() const { return m_fSkillListOffsetX; }

    private:
        void LoadImages();
        void UnloadImages();

        void RenderCenterRegion();
        void RenderLeftFrame();
        void RenderCenterFrame();

        void SyncRmlModel();
        void BuildRmlUi();

    public:
        __int64	m_loPreExp;
        __int64	m_loGetExp;

    private:
        CManager* m_pNewUIMng;
        C3DRenderMng* m_pNewUI3DRenderMng;

        CItemHotKey m_ItemHotKey;

        bool m_bExpEffect;
        DWORD m_dwExpEffectTime;

        __int64 m_dwPreExp;
        __int64 m_dwGetExp;

        // See GetItemHotkeyOffsetX()/GetSkillListOffsetX() above. Recomputed every frame in
        // SyncRmlModel(); 0.f until the RmlUi doc exists.
        float m_fItemHotkeyOffsetX = 0.f;
        float m_fSkillListOffsetX = 0.f;

        bool m_bButtonBlink;

        struct MainFrameRmlModel
        {
            // #bars/#buttons/#exp share one transform group, bound every frame from
            // UI::Scaling::BottomHudCenterTransform() (clamped 1x-2x scale, folds in
            // GetUIScalePercent()) so it tracks the still-legacy center-band chrome exactly.
            // Buttons and exp are nested inside #bars in main_frame.rml to inherit this transform
            // rather than each needing their own binding. Lengths in main_frame.rcss stay `px`
            // (not `dp`) so they scale via bars_scale only, not a second time via RmlUi's
            // density-independent-pixel ratio.
            float barsLeft = 0.f, barsTop = 0.f, barsScale = 1.f;

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

            // Cooldown wipe for the compact row + current-skill slot; same per-field convention as skillSlot0..4Hotkey.
            float skillSlot0Cooldown = 0.f, skillSlot1Cooldown = 0.f, skillSlot2Cooldown = 0.f,
                  skillSlot3Cooldown = 0.f, skillSlot4Cooldown = 0.f;
            float currentSkillCooldown = 0.f;

            // Mirrors CSkillList::IsSkillGridOpen() (NOT the similarly-named IsSkillListUp() --
            // see that method's comment). Gates #skill_grid/#pet_skill_row visibility; cell arrays
            // are rebuilt each frame while open, left stale (harmless, hidden) while closed.
            bool skillGridOpen = false;
            std::vector<SkillCellEntry> skillGridCells;
            std::vector<SkillCellEntry> petSkillCells;

            // Item hotkey chrome (#item_slots): hover border + stack-count for the 4 Q/W/E/R
            // slots. The 3D-rendered potion icon itself is untouched (permanent native content).
            bool itemSlot0Hovered = false, itemSlot1Hovered = false, itemSlot2Hovered = false, itemSlot3Hovered = false;
            Rml::String itemSlot0Count, itemSlot1Count, itemSlot2Count, itemSlot3Count;
        };
        RmlModelBinder<MainFrameRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // The left/center HUD-strip background must render BEHIND the legacy 3D-composited
        // item/skill icons, which always paint after m_pRmlDoc's "main" context -- so it lives in
        // a separate RmlUiRuntime::GetBackgroundContext() document/model, painted by
        // CManager::Render()'s centralized RenderBackgroundLayer() call.
        struct MainFrameBgRmlModel
        {
            // Mirrors MainFrameRmlModel::barsLeft/barsTop/barsScale (#bg_root uses the same
            // data-style-left/top/scale convention as #bars).
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;

            // Same offsets as GetItemHotkeyOffsetX()/GetSkillListOffsetX(), added on top of rootX
            // in main_frame_bg.rml so the two panels track their real anchors.
            float leftOffsetX = 0.f, centerOffsetX = 0.f;
        };
        RmlModelBinder<MainFrameBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

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
